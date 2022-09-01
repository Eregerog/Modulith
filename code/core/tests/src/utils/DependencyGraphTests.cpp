/*
 * \brief
 * \author Daniel Götz
 */


#include "catch/catch.hpp"

#define MODU_THROW_ON_ASSERTS

#include "utils/DependencyGraph.h"

using namespace modulith;

SCENARIO("Items can be added to the graph") {
    GIVEN("An empty dependency graph") {
        auto graph = DependencyGraph<int>();

        REQUIRE(graph.Count() == 0);
        REQUIRE_FALSE(graph.Contains(42));

        WHEN("An item is added") {
            graph.Add(42);

            THEN("The count is increased") {
                REQUIRE(graph.Count() == 1);
            }

            THEN("It is contained") {
                REQUIRE(graph.Contains(42));
            }

            THEN("It is a start and end node") {
                REQUIRE(graph.IsStart(42));
                REQUIRE(graph.IsEnd(42));
            }

            THEN("It has no dependencies") {
                REQUIRE_FALSE(graph.HasPrev(42));
                REQUIRE_FALSE(graph.HasNext(42));
            }

            AND_WHEN("Another item is added") {
                graph.Add(666);

                THEN("The count is increased") {
                    REQUIRE(graph.Count() == 2);
                }

                THEN("It is contained") {
                    REQUIRE(graph.Contains(666));
                }

                THEN("It is a start and end node") {
                    REQUIRE(graph.IsStart(666));
                    REQUIRE(graph.IsEnd(666));
                }

                THEN("It has no dependencies") {
                    REQUIRE_FALSE(graph.HasPrev(666));
                    REQUIRE_FALSE(graph.HasNext(666));
                }

                THEN("The two contained items have no dependencies between each other") {
                    REQUIRE_FALSE(graph.IsAnyPrevOf(42, 666));
                    REQUIRE_FALSE(graph.IsAnyNextOf(42, 666));

                    REQUIRE_FALSE(graph.IsAnyPrevOf(666, 42));
                    REQUIRE_FALSE(graph.IsAnyNextOf(666, 42));
                }
            }

            AND_WHEN("The item is inserted again") {
                graph.Add(42);

                THEN("The count is not increased") {
                    REQUIRE(graph.Count() == 1);
                }

                THEN("It is still contained") {
                    REQUIRE(graph.Contains(42));
                }

                THEN("It is still a start and end node") {
                    REQUIRE(graph.IsStart(42));
                    REQUIRE(graph.IsEnd(42));
                }

                THEN("It still has no dependencies") {
                    REQUIRE_FALSE(graph.HasPrev(42));
                    REQUIRE_FALSE(graph.HasNext(42));
                }
            }
        }
    }
}

SCENARIO("Dependencies between items can be created") {
    GIVEN("A dependency graph with 3 items") {
        auto graph = DependencyGraph<int>({69, 420, 1337});

        REQUIRE(graph.Count() == 3);

        REQUIRE(graph.IsStart(69));
        REQUIRE(graph.IsStart(420));
        REQUIRE(graph.IsStart(1337));

        REQUIRE(graph.IsEnd(69));
        REQUIRE(graph.IsEnd(420));
        REQUIRE(graph.IsEnd(1337));

        THEN("Dependencies between nodes that are not inserted cannot be added") {
            REQUIRE_FALSE(graph.CanAddDependency(42, 69));
            REQUIRE_FALSE(graph.CanAddDependency(69, 42));
        }

        THEN("Dependencies between a node itself cannot be added") {
            REQUIRE_FALSE(graph.CanAddDependency(69, 69));
        }

        WHEN("A dependency between two nodes is added") {
            REQUIRE(graph.CanAddDependency(69, 420));

            graph.AddDependency(69, 420);

            THEN("The next of and prev ofs are properly set") {
                REQUIRE(graph.HasNext(69));
                REQUIRE_FALSE(graph.HasNext(420));
                REQUIRE(graph.HasPrev(420));
                REQUIRE_FALSE(graph.HasPrev(69));

                REQUIRE(graph.IsDirectPrevOf(420, 69));
                REQUIRE(graph.IsAnyPrevOf(420, 69));
                REQUIRE_FALSE(graph.IsIndirectPrevOf(420, 69));

                REQUIRE(graph.IsDirectNextOf(69, 420));
                REQUIRE(graph.IsAnyNextOf(69, 420));
                REQUIRE_FALSE(graph.IsIndirectNextOf(69, 420));
            }

            THEN("The node start and end status is changed") {
                REQUIRE(graph.IsStart(69));
                REQUIRE_FALSE(graph.IsStart(420));

                REQUIRE_FALSE(graph.IsEnd(69));
                REQUIRE(graph.IsEnd(420));
            }

            THEN("A circular dependency may not be created") {
                REQUIRE_FALSE(graph.CanAddDependency(420, 69));
            }

            AND_WHEN("Another dependency to another node is added after the last") {
                REQUIRE(graph.CanAddDependency(420, 1337));

                graph.AddDependency(420, 1337);

                THEN("The next of and prev ofs are properly set") {
                    REQUIRE(graph.HasNext(420));
                    REQUIRE_FALSE(graph.HasNext(1337));
                    REQUIRE(graph.HasPrev(1337));
                    REQUIRE(graph.HasPrev(420));

                    REQUIRE(graph.IsDirectPrevOf(1337, 420));
                    REQUIRE(graph.IsAnyPrevOf(1337, 420));
                    REQUIRE_FALSE(graph.IsIndirectPrevOf(1337, 420));

                    REQUIRE(graph.IsDirectNextOf(420, 1337));
                    REQUIRE(graph.IsAnyNextOf(420, 1337));
                    REQUIRE_FALSE(graph.IsIndirectNextOf(420, 1337));
                }

                THEN("The node start and end status is changed") {
                    REQUIRE_FALSE(graph.IsStart(420));
                    REQUIRE_FALSE(graph.IsStart(1337));

                    REQUIRE_FALSE(graph.IsEnd(420));
                    REQUIRE(graph.IsEnd(1337));
                }

                THEN("A circular dependency may not be created") {
                    REQUIRE_FALSE(graph.CanAddDependency(1337, 420));
                }

                THEN("There are proper indirect dependencies between the first and last node") {
                    REQUIRE_FALSE(graph.IsDirectPrevOf(1337, 69));
                    REQUIRE(graph.IsAnyPrevOf(1337, 69));
                    REQUIRE(graph.IsIndirectPrevOf(1337, 69));

                    REQUIRE_FALSE(graph.IsDirectNextOf(69, 1337));
                    REQUIRE(graph.IsAnyNextOf(69, 1337));
                    REQUIRE(graph.IsIndirectNextOf(69, 1337));
                }
            }
        }
    }
}

SCENARIO("Dependencies can be removed") {
    GIVEN("A dependency graph with 5 items that have dependencies") {
        auto graph = DependencyGraph<int>({69, 100, 420, 666, 1337});
        graph.AddDependency(69, 100);
        graph.AddDependency(69, 420);
        graph.AddDependency(69, 666);
        graph.AddDependency(420, 1337);

        THEN("Indirect dependencies cannot be removed") {
            REQUIRE_FALSE(graph.CanRemoveDependency(69, 1337));
        }

        THEN("Dependencies between nodes that are not inserted cannot be removed") {
            REQUIRE_FALSE(graph.CanRemoveDependency(42, 69));
            REQUIRE_FALSE(graph.CanRemoveDependency(69, 42));
        }

        THEN("Dependencies between a node itself cannot be removed") {
            REQUIRE_FALSE(graph.CanRemoveDependency(69, 69));
        }

        WHEN("The first dependency is removed") {
            REQUIRE(graph.CanRemoveDependency(69, 420));

            graph.RemoveDependency(69, 420);

            THEN("The next of and prev ofs are properly reset") {
                REQUIRE(graph.HasNext(69));
                REQUIRE(graph.HasNext(420));
                REQUIRE_FALSE(graph.HasPrev(420));
                REQUIRE_FALSE(graph.HasPrev(69));

                REQUIRE_FALSE(graph.IsDirectPrevOf(420, 69));
                REQUIRE_FALSE(graph.IsAnyPrevOf(420, 69));
                REQUIRE_FALSE(graph.IsIndirectPrevOf(420, 69));

                REQUIRE_FALSE(graph.IsDirectNextOf(69, 420));
                REQUIRE_FALSE(graph.IsAnyNextOf(69, 420));
                REQUIRE_FALSE(graph.IsIndirectNextOf(69, 420));
            }

            THEN("The node start and end status is changed") {
                REQUIRE(graph.IsStart(69));
                REQUIRE(graph.IsStart(420));

                REQUIRE_FALSE(graph.IsEnd(69));
                REQUIRE_FALSE(graph.IsEnd(420));
            }

            THEN("The dependency cannot be removed again") {
                REQUIRE_FALSE(graph.CanRemoveDependency(69, 420));
            }

            AND_WHEN("The second dependency is removed") {
                REQUIRE(graph.CanRemoveDependency(420, 1337));

                graph.RemoveDependency(420, 1337);

                THEN("The next of and prev ofs are properly reset") {
                    REQUIRE_FALSE(graph.HasNext(420));
                    REQUIRE_FALSE(graph.HasNext(1337));
                    REQUIRE_FALSE(graph.HasPrev(1337));
                    REQUIRE_FALSE(graph.HasPrev(420));

                    REQUIRE_FALSE(graph.IsDirectPrevOf(1337, 420));
                    REQUIRE_FALSE(graph.IsAnyPrevOf(1337, 420));
                    REQUIRE_FALSE(graph.IsIndirectPrevOf(1337, 420));

                    REQUIRE_FALSE(graph.IsDirectNextOf(420, 1337));
                    REQUIRE_FALSE(graph.IsAnyNextOf(420, 1337));
                    REQUIRE_FALSE(graph.IsIndirectNextOf(420, 1337));
                }

                THEN("The node start and end status is changed") {
                    REQUIRE(graph.IsStart(420));
                    REQUIRE(graph.IsStart(1337));

                    REQUIRE(graph.IsEnd(420));
                    REQUIRE(graph.IsEnd(1337));
                }

                THEN("The dependency cannot be removed again") {
                    REQUIRE_FALSE(graph.CanRemoveDependency(420, 1337));
                }

                THEN("There indirect dependencies between the first and last node are removed") {
                    REQUIRE_FALSE(graph.IsAnyPrevOf(1337, 69));
                    REQUIRE_FALSE(graph.IsIndirectPrevOf(1337, 69));

                    REQUIRE_FALSE(graph.IsAnyNextOf(69, 1337));
                    REQUIRE_FALSE(graph.IsIndirectNextOf(69, 1337));
                }

                AND_WHEN("All remaining dependencies are removed") {
                    REQUIRE(graph.CanRemoveDependency(69, 100));
                    REQUIRE(graph.CanRemoveDependency(69, 666));

                    graph.RemoveDependency(69, 100);
                    graph.RemoveDependency(69, 666);

                    THEN("All nodes are independent") {
                        REQUIRE(graph.IsStart(69));
                        REQUIRE(graph.IsStart(100));
                        REQUIRE(graph.IsStart(420));
                        REQUIRE(graph.IsStart(666));
                        REQUIRE(graph.IsStart(1337));

                        REQUIRE(graph.IsEnd(69));
                        REQUIRE(graph.IsEnd(100));
                        REQUIRE(graph.IsEnd(420));
                        REQUIRE(graph.IsEnd(666));
                        REQUIRE(graph.IsEnd(1337));
                    }

                    THEN("All nodes have no nexts / prevs") {
                        REQUIRE_FALSE(graph.HasPrev(1337));
                        REQUIRE_FALSE(graph.HasPrev(666));
                        REQUIRE_FALSE(graph.HasPrev(420));
                        REQUIRE_FALSE(graph.HasPrev(100));
                        REQUIRE_FALSE(graph.HasPrev(69));

                        REQUIRE_FALSE(graph.HasNext(1337));
                        REQUIRE_FALSE(graph.HasNext(666));
                        REQUIRE_FALSE(graph.HasNext(420));
                        REQUIRE_FALSE(graph.HasNext(100));
                        REQUIRE_FALSE(graph.HasNext(69));
                    }
                }
            }
        }
    }
}

SCENARIO("Items can be removed from the graph") {
    GIVEN("A dependency graph with 3 items") {
        auto graph = DependencyGraph<int>({69, 420, 666});

        REQUIRE(graph.Count() == 3);

        WHEN("An item is removed") {
            graph.Remove(69);

            THEN("Is is no longer contained") {
                REQUIRE_FALSE(graph.Contains(69));
                REQUIRE(graph.Count() == 2);
            }
        }
    }

    GIVEN("A dependency graph with 5 items that have dependencies") {
        auto graph = DependencyGraph<int>({69, 100, 420, 666, 1337});
        graph.AddDependency(69, 100);
        graph.AddDependency(69, 420);
        graph.AddDependency(420, 666);
        graph.AddDependency(420, 1337);


        WHEN("An item with dependencies is removed") {
            graph.Remove(420);

            THEN("The nexts and prevs are properly set") {
                REQUIRE_FALSE(graph.HasPrev(666));
                REQUIRE_FALSE(graph.HasPrev(1337));

                REQUIRE(graph.NextsOf(69).size() == 1);
            }

            THEN("Nodes are properly made start / end nodes") {
                REQUIRE(graph.IsStart(666));
                REQUIRE(graph.IsStart(1337));

                // because 100 still comes after
                REQUIRE_FALSE(graph.IsEnd(69));
            }
        }
    }

}

template<class T>
int indexOf(std::vector<T> list, T item) {
    for (int i = 0; i < list.size(); ++i)
        if (list[i] == item)
            return i;
    return -1;
}

template<class T>
bool firstComesBeforeSecond(std::vector<T> list, T before, T after) {
    return indexOf<T>(list, before) < indexOf<T>(list, after);
}

template<class T>
bool contains(std::vector<T> list, T item) { return indexOf<T>(list, item) >= 0; }

SCENARIO("The graph can be topologically traversed") {
    GIVEN("A dependency graph with a couple of interconnected items") {
        auto graph = DependencyGraph<int>({01, 02, 03, 10, 20, 21, 22, 30, 31, 42});

        graph.AddDependency(01, 10);
        graph.AddDependency(02, 10);

        graph.AddDependency(03, 21);
        graph.AddDependency(03, 22);

        graph.AddDependency(10, 20);
        graph.AddDependency(10, 21);

        graph.AddDependency(20, 30);
        graph.AddDependency(21, 31);
        graph.AddDependency(22, 31);

        graph.AddDependency(31, 42);
        graph.AddDependency(22, 42);

        WHEN("TopTraversing from an end node to the start") {
            auto res = graph.AllNodesFromNodeToStartTopological(42);
            REQUIRE(res.size() == 8);

            THEN("The result has the correct order") {
                REQUIRE(indexOf(res, 42) == 0);

                REQUIRE_FALSE(contains(res, 20));
                REQUIRE_FALSE(contains(res, 30));

                REQUIRE(firstComesBeforeSecond(res, 42, 31));
                REQUIRE(firstComesBeforeSecond(res, 42, 22));
                REQUIRE(firstComesBeforeSecond(res, 31, 22));

                REQUIRE(firstComesBeforeSecond(res, 31, 21));
                REQUIRE(firstComesBeforeSecond(res, 22, 3));
                REQUIRE(firstComesBeforeSecond(res, 21, 3));

                REQUIRE(firstComesBeforeSecond(res, 21, 10));
                REQUIRE(firstComesBeforeSecond(res, 10, 1));
                REQUIRE(firstComesBeforeSecond(res, 10, 2));
            }
        }

        WHEN("TopTraversing from a start node to the end") {
            auto res = graph.AllNodesFromNodeToEndTopological(3);
            REQUIRE(res.size() == 5);

            THEN("The result has the correct order") {
                REQUIRE(indexOf(res, 3) == 0);

                REQUIRE(firstComesBeforeSecond(res, 3, 21));
                REQUIRE(firstComesBeforeSecond(res, 3, 22));
                REQUIRE(firstComesBeforeSecond(res, 21, 31));
                REQUIRE(firstComesBeforeSecond(res, 22, 31));

                REQUIRE(firstComesBeforeSecond(res, 31, 42));
                REQUIRE(firstComesBeforeSecond(res, 22, 42));
            }
        }
    }
}

SCENARIO("The module dependencies can be represented and queried with the dependency graph") {
    GIVEN("A dependency graph which represents the loaded modules") {
        auto graph = DependencyGraph<char>({'R', 'P', 'D', 'G', 'S', 'M'});

        graph.AddDependency('R', 'D');

        graph.AddDependency('R', 'G');
        graph.AddDependency('P', 'G');

        graph.AddDependency('R', 'S');
        graph.AddDependency('P', 'S');
        graph.AddDependency('G', 'S');

        graph.AddDependency('R', 'M');
        graph.AddDependency('P', 'M');
        graph.AddDependency('G', 'M');
        graph.AddDependency('S', 'M');

        WHEN("TopTraversing from an the physics module to the end") {
            auto res = graph.AllNodesFromNodeToEndTopological('P');
            REQUIRE(res.size() == 4);

            THEN("The result contains all modules dependant on the physics module, in topological order") {
                REQUIRE(indexOf(res, 'P') == 0);
                REQUIRE(indexOf(res, 'G') == 1);
                REQUIRE(indexOf(res, 'S') == 2);
                REQUIRE(indexOf(res, 'M') == 3);
            }
        }
    }
}


SCENARIO("The graph can be cloned") {

    GIVEN("A dependency graph with a couple of interconnected items") {
        auto graph = DependencyGraph<int>({1, 2, 3});

        graph.AddDependency(1, 2);
        graph.AddDependency(2, 3);

        WHEN("The graph is cloned") {
            auto clone = DependencyGraph<int>(graph);

            THEN("They contain the same nodes and dependencies") {
                REQUIRE(clone.Count() == 3);

                REQUIRE(clone.Contains(1));
                REQUIRE(clone.Contains(2));
                REQUIRE(clone.Contains(3));


                REQUIRE(clone.IsDirectNextOf(1, 2));
                REQUIRE(clone.IsDirectNextOf(2, 3));
                REQUIRE(clone.IsIndirectNextOf(1, 3));


                REQUIRE(clone.IsDirectPrevOf(2, 1));
                REQUIRE(clone.IsDirectPrevOf(3, 2));
                REQUIRE(clone.IsIndirectPrevOf(3, 1));
            }

            AND_WHEN("The original graph is modified by removing a node") {
                graph.Remove(3);
                REQUIRE_FALSE(graph.Contains(3));

                THEN("The cloned graph is not affected") {
                    REQUIRE(clone.Contains(3));

                    REQUIRE(clone.IsDirectNextOf(2, 3));
                    REQUIRE(clone.IsIndirectNextOf(1, 3));

                    REQUIRE(clone.IsDirectPrevOf(3, 2));
                    REQUIRE(clone.IsIndirectPrevOf(3, 1));
                }
            }

            AND_WHEN("The original graph is modified by removing a dependency") {
                graph.RemoveDependency(2, 3);
                REQUIRE_FALSE(graph.IsDirectNextOf(2, 3));
                REQUIRE_FALSE(graph.IsDirectPrevOf(3, 2));

                THEN("The cloned graph is not affected") {

                    REQUIRE(clone.IsDirectNextOf(2, 3));
                    REQUIRE(clone.IsIndirectNextOf(1, 3));

                    REQUIRE(clone.IsDirectPrevOf(3, 2));
                    REQUIRE(clone.IsIndirectPrevOf(3, 1));
                }
            }
        }

    }

}