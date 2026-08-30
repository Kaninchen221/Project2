#include "ECS/P2Schedule.hpp"

#include "P2Clock.hpp"

#include <ranges>
#include <future>

namespace P2::ecs
{
	void Schedule::buildGraph()
	{
		// Create graph nodes from systems infos
		auto& graphNodes = graph.nodes;
		for (const auto& system : systems)
		{
			GraphNode graphNode
			{
				.typeID = system.label,
				.typeInfo = system.typeInfo,
				.systemAdapter = system.systemAdapter,
				.after = system.after,
				.before = system.before,
				.mainThread = system.mainThread,
				.resources = system.resources
			};

			for (const auto& otherSystem : systems)
			{
				if (otherSystem.label == graphNode.typeID)
					continue;

				if (std::ranges::contains(otherSystem.after, graphNode.typeID))
					graphNode.before.emplace_back(otherSystem.label);

				if (std::ranges::contains(otherSystem.before, graphNode.typeID))
					graphNode.after.emplace_back(otherSystem.label);
			}

			graphNodes.push_back(graphNode);
		}

		auto& edges = graph.edges;

		// Create edges from 'Before' and 'After'
		for (const auto& system : systems)
		{
			for (auto& before : system.before)
			{
				GraphEdge graphEdge
				{
					.from = system.label,
					.to = before
				};

				if (!std::ranges::contains(edges, graphEdge))
					edges.push_back(graphEdge);
			}

			for (auto& after : system.after)
			{
				GraphEdge graphEdge
				{
					.from = after,
					.to = system.label
				};

				if (!std::ranges::contains(edges, graphEdge))
					edges.push_back(graphEdge);
			}
		}

		// Create edges from resources: type and method of use (ReadWrite or ReadOnly)
		for (const auto& system : systems)
		{
			for (const auto& resource : system.resources)
			{
				// Don't create edges that has 'from' const resource usage
				if (resource.isConst)
					continue;

				for (const auto& otherSystem : systems)
				{
					// Skip the same system
					if (system.label == otherSystem.label)
						continue;

					auto& otherResources = otherSystem.resources;
					if (std::ranges::any_of(otherResources, [&resource](const auto& otherResource) { return resource.type == otherResource.type; }))
					{
						GraphEdge graphEdge
						{
							.from = system.label,
							.to = otherSystem.label
						};

						// Skip edge if it's already in the opposite direction
						auto graphEdgePrediction = [&graphEdge](const auto& otherEdge)
						{
							return (otherEdge.from == graphEdge.from && otherEdge.to == graphEdge.to) ||
								(otherEdge.from == graphEdge.to && otherEdge.to == graphEdge.from);
						};

						if (!std::ranges::any_of(edges, graphEdgePrediction))
						{
							edges.push_back(graphEdge);
						}
					}
				}
			}
		}

		// Create edges from queries: types and method of use (ReadWrite or ReadOnly)
		for (const auto& system : systems)
		{
			for (const auto& query : system.queries)
			{
				// Don't create edges that has 'from' const query usage
				if (query.isConst)
					continue;

				for (const auto& otherSystem : systems)
				{
					// Skip the same system
					if (system.label == otherSystem.label)
						continue;

					// If other query contains the same collection of types as in query then add edge
					// It means that the other query can have additional types
					auto& otherQueries = otherSystem.queries;
					auto queryHasTypesPrediction = [&query = query](const auto& otherQuery)
					{
						for (auto type : query.types)
						{
							if (!std::ranges::contains(otherQuery.types, type))
								return false;
						}

						return true;
					};

					if (std::ranges::any_of(otherQueries, queryHasTypesPrediction))
					{
						GraphEdge graphEdge
						{
							.from = system.label,
							.to = otherSystem.label
						};

						// Skip edge if it's already in the opposite direction
						auto graphEdgePrediction = [&graphEdge = graphEdge](const auto& otherEdge)
						{
							return (otherEdge.from == graphEdge.from && otherEdge.to == graphEdge.to) ||
								(otherEdge.from == graphEdge.to && otherEdge.to == graphEdge.from);
						};

						if (!std::ranges::any_of(edges, graphEdgePrediction))
						{
							edges.push_back(graphEdge);
						}
					}
				}
			}
		}
	}

	void Schedule::resolveGraph()
	{
		auto& notSortedNodes = graph.nodes;
		std::vector<GraphNode> nodesWithoutIncomingEdge;
		auto& layers = graph.layers;
		auto& edges = graph.edges;

		auto hasIncomingEdge = [&edges = edges](const GraphNode& node) -> bool
		{
			for (const auto& edge : edges)
			{
				if (edge.to == node.typeID)
					return true;
			}

			return false;
		};

		// Kahn's algorithm
		do
		{
			nodesWithoutIncomingEdge.clear();

			// First find all nodes that doesn't have an incoming edge
			for (size_t i = 0; i < notSortedNodes.size(); ++i)
			{
				const auto& node = notSortedNodes[i];
				if (!hasIncomingEdge(node))
				{
					nodesWithoutIncomingEdge.push_back(node);
					notSortedNodes.erase(notSortedNodes.begin() + i);
					--i;
				}
			}

			if (nodesWithoutIncomingEdge.empty())
				break;

			layers.push_back(
				GraphLayer
				{
					.nodes = nodesWithoutIncomingEdge
				}
			);

			// Remove edges
			for (const auto& nodeWithoutIncomingEdge : nodesWithoutIncomingEdge)
			{
				for (size_t i = 0; i < edges.size(); ++i)
				{
					auto& edge = edges[i];
					if (edge.from == nodeWithoutIncomingEdge.typeID)
					{
						edges.erase(edges.begin() + i);
						--i;
					}
				}
			}
		} while (!nodesWithoutIncomingEdge.empty());

		// Sort the nodes in layers because of the MainThread dependency
		for (auto& layer : layers)
		{
			auto& nodes = layer.nodes;
			std::sort(nodes.begin(), nodes.end(),
				[](const auto& first, const auto& second) -> bool
			{
				return first.mainThread < second.mainThread;
			}
			);
		}

		/// Create taskflow per layer
		for (auto& layer : layers)
		{
			for (auto& node : layer.nodes)
			{
				if (node.mainThread)
				{
					continue;
				}

				layer.taskflow.emplace(
					[&node = node, getWorld = std::bind(&Schedule::getWorld, this)]()
					{
						auto& world = std::invoke(getWorld);

						if (ShouldSkipNode(node, world))
						{
							Logger->warn("Skip node: {}", node.typeInfo->name());
							return;
						}

#					if P2_TIME_TRACE
						Clock clock;
#					endif

						auto& systemAdapter = node.systemAdapter;
						if (systemAdapter)
						{
							std::invoke(systemAdapter, world);
						}

#					if P2_TIME_TRACE
						node.executeTime = clock.getElapsedTime();
						Logger->trace("Executing node: {} took: {}us", node.typeInfo->name(), node.executeTime.getAsMicroseconds().count());
#					endif
					}
				);
			}
		}
	}

	void Schedule::runOnce(World& world)
	{
		worldPtr = &world;

		auto& layers = graph.layers;
		for (auto& layer : layers)
		{
			{
				/// Run async nodes
				auto asyncResult = executor.run(layer.taskflow);

				/// Run main thread nodes
				for (auto& node : layer.nodes)
				{
					if (ShouldSkipNode(node, world))
					{
						Logger->warn("Skip node: {}", node.typeInfo->name());
						continue;
					}

					if (node.mainThread)
					{
#					if P2_TIME_TRACE
						Clock clock;
#					endif

						std::invoke(node.systemAdapter, world);

#					if P2_TIME_TRACE
						node.executeTime = clock.getElapsedTime();
						Logger->trace("Executing node: {} took: {}us", node.typeInfo->name(), node.executeTime.getAsMicroseconds().count());
#					endif
					}
				}

				asyncResult.wait();
			}

			world.executeCommands();
			world.clearCommands();
		}
	}

	bool Schedule::ShouldSkipNode(const GraphNode& node, const World& world) noexcept
	{
		for (const auto& resource : node.resources)
		{
			if (!world.hasResource(resource.type))
			{
				Logger->info("Couldn't find a resource with the typeID: {} in the world", 
					resource.type);
				return true;
			}
		}

		return false;
	}
}