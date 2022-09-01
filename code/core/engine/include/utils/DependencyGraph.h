/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"

namespace modulith{

    /**
     * This class can be used to represent a cycle-free and directed graph of nodes.
     * Start nodes are nodes without any incoming edge.
     * End nodes are nodes without any outgoing edge.
     * Nodes meeting either of these conditions are added to the start / end nodes automatically.
     * @tparam T The type of the node
     * @tparam Hasher A struct to create a hashcode for a node
     * @tparam EqualTo A struct that can be used to compare two node types
     */
    template<class T, class Hasher = std::hash<T>, class EqualTo = std::equal_to<T>>
    class DependencyGraph {

        using ValueSet = std::unordered_set<T, Hasher, EqualTo>;
        using DependencyMultiMap = std::unordered_multimap<T, T, Hasher, EqualTo>;

    public:

        /**
         * Creates a graph with no nodes or edges
         */
        DependencyGraph() = default;

        /**
         * Creates a graph with a specific amount of starting nodes with no edges
         */
        DependencyGraph(std::initializer_list<T> startingNodes) {
            for (auto& node : startingNodes)
                Add(node);
        }

        /**
         * Creates a shallow-copy from the given graph
         */
        DependencyGraph(const DependencyGraph<T, Hasher, EqualTo>& toClone) {
            _nodes = ValueSet(toClone._nodes);
            _startNodes = ValueSet(toClone._startNodes);
            _endNodes = ValueSet(toClone._endNodes);

            _prevs = DependencyMultiMap(toClone._prevs);
            _nexts = DependencyMultiMap(toClone._nexts);
        }

        /**
        * @name Node queries / manipulation
        * Contains methods for querying the nodes in the graph and for adding / removing nodes
        */
        ///@{

        /**
         * @return Returns the amount of nodes in the graph
         */
        [[nodiscard]]
        size_t Count() const { return _nodes.size(); }

        /**
         * @return Returns whether the given node is contained inside the graph
         */
        [[nodiscard]]
        bool Contains(const T& item) const { return _nodes.count(item) > 0; }

        /**
         * Adds the given node to the graph. Will do nothing if it is already contained
         */
        void Add(T item) {
            if (!Contains(item)) {
                _nodes.insert(item);
                _startNodes.insert(item);
                _endNodes.insert(item);
            }
        }

        /**
         * Removes the given node from the graph. Will do nothing if the nodes is not contained
         */
        void Remove(const T& item) {
            if (Contains(item)) {
                for (const auto& prev : PrevsOf(item))
                    RemoveDependency(prev, item);
                for (const auto& next : NextsOf(item))
                    RemoveDependency(item, next);
                _nodes.erase(item);
                _startNodes.erase(item);
                _endNodes.erase(item);
            }
        }

        /**
         * Removes all nodes and their edges from the graph
         */
        void Clear() {
            for (const auto& item : ValueSet(_nodes))
                Remove(item);
        }

        /**
         * @return If the given node is a start node, which is a node with no incoming edges
         */
        [[nodiscard]]
        bool IsStart(const T& item) const {
            CoreAssert(Contains(item), "IsStart was called for an item that is not part of the dependency graph")
            return _startNodes.count(item) > 0;
        }

        /**
         * @return If the given node is an end node, which is a node with no outcoming edges
         */
        [[nodiscard]]
        bool IsEnd(const T& item) const {
            CoreAssert(Contains(item), "IsEnd was called for an item that is not part of the dependency graph")
            return _endNodes.count(item) > 0;
        }

        ///@}

        /**
         * @return Returns a set of copies of all nodes contain in this graph graph
         */
        [[nodiscard]] ValueSet AllNodes() const { return _nodes; }

        /**
         * @return Returns a set of copies of all start nodes of this graph
         */
        [[nodiscard]] ValueSet StartNodes() const { return _startNodes; }

        /**
         * @return Returns a set of copies of all end nodes of this graph
         */
        [[nodiscard]] ValueSet EndNodes() const { return _endNodes; }

        /**
          * @name Dependency manipulation
          * Contains methods for adding / removing dependencies between nodes
          */
        ///@{

        /**
         * Returns whether an edge can be added between two given nodes.
         * Edges can not be added if they would cause a cycle or one of the nodes is not part of the graph.
         * @param from The node the edge starts at 
         * @param to The node the edge ends at
         */
        [[nodiscard]]
        bool CanAddDependency(const T& from, const T& to) {
            return Contains(from) && Contains(to) && !EqualTo()(from, to) && !IsAnyPrevOf(from, to);
        }

        /**
         * Returns whether an edge can be removed between two given nodes.
         * Edges can no be removed if it does not exist or either of the nodes are not contained
         * @param from The node the edge starts at 
         * @param to The node the edge ends at
         */
        [[nodiscard]]
        bool CanRemoveDependency(const T& from, const T& to) {
            return Contains(from) && Contains(to) && !EqualTo()(from, to) && IsDirectNextOf(from, to);
        }

        /**
         * Adds an edge between the given nodes. The edge insertion must be valid, 
         * which can be check using {@link CanAddDependency}
         * @param from The node the edge starts at 
         * @param to The node the edge ends at
         */
        void AddDependency(const T& from, const T& to) {
            CoreAssert(CanAddDependency(from, to),
                "AddDependency was called for two nodes where a dependency could not be added")

            _nexts.emplace(from, to);
            _prevs.emplace(to, from);
            _startNodes.erase(to);
            _endNodes.erase(from);
        }

        /**
         * Removes an edge between the given nodes. The edge removal must be valid,
         * which can be check using {@link CanRemoveDependency}
         * @param from The node the edge starts at 
         * @param to The node the edge ends at
         */
        void RemoveDependency(const T& from, const T& to) {
            CoreAssert(CanRemoveDependency(from, to),
                "RemoveDependency was called for two nodes where a dependency could not be removed")

            eraseKeyValuePair(_nexts, from, to);
            eraseKeyValuePair(_prevs, to, from);
            if (!HasNext(from))
                _endNodes.insert(from);
            if (!HasPrev(to))
                _startNodes.insert(to);
        }

        ///@}
        
        /**
         * @name Prev / Next queries
         * This section contains methods for checking the relation of two nodes.
         * A node is a "prev" of another, if it has an outgoing edge to it.
         * It is the opposite with "next". 
         */
        ///@{

        /**
         * @param item A node that is contained in the graph
         * @return If the given node has any direct prevs
         */
        [[nodiscard]]
        bool HasPrev(const T& item) const {
            CoreAssert(Contains(item), "Could not get the next of since the item is not contained in the sequence!");
            return _prevs.count(item) > 0;
        }
        
        /**
         * @param item A node that is contained in the graph
         * @return If the given node has any direct nexts
         */
        [[nodiscard]]
        bool HasNext(const T& item) const {
            CoreAssert(Contains(item), "Could not get the next of since the item is not contained in the sequence!");
            return _nexts.count(item) > 0;
        }

       
        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the prev node is a direct previous node of the base node
         */
        [[nodiscard]]
        bool IsDirectPrevOf(const T& base, const T& prev) const {
            auto prevs = PrevsOf(base);
            for(auto& item : prevs){
                if(EqualTo()(item, prev))
                    return true;
            }
            return false;
        }

        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the prev node is an indirect previous node of the base node but not a direct one
         */
        [[nodiscard]]
        bool IsIndirectPrevOf(const T& base, const T& next) const {
            return !IsDirectPrevOf(base, next) && IsAnyPrevOf(base, next);
        }

        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the prev node is a direct or indirect previous node of the base node
         */
        [[nodiscard]]
        bool IsAnyPrevOf(const T& base, const T& prev) const {
            auto prevs = AllPrevsOf(base);
            for(auto& item : prevs){
                if(EqualTo()(item, prev))
                    return true;
            }
            return false;
        }

        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the next node is a direct next node of the base node
         */
        [[nodiscard]]
        bool IsDirectNextOf(const T& base, const T& next) const {
            auto nexts = NextsOf(base);
            for(auto& item : nexts){
                if(EqualTo()(item, next))
                    return true;
            }
            return false;
        }

        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the next node is an indirect next node of the base node but not a direct one
         */
        [[nodiscard]]
        bool IsIndirectNextOf(const T& base, const T& next) const {
            return !IsDirectNextOf(base, next) && IsAnyNextOf(base, next);
        }
        
        /**
         * @param base A node that must be contained in the graph
         * @param prev A node that is contained in the graph
         * @return If the next node is a direct or indirect next node of the base node
         */
        [[nodiscard]]
        bool IsAnyNextOf(const T& base, const T& next) const {
            auto nexts = AllNextsOf(base);
            for(auto& item : nexts){
                if(EqualTo()(item, next))
                    return true;
            }
            return false;
        }

        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all the nodes that are direct previous nodes of the given one
         */
        [[nodiscard]]
        std::vector<T> PrevsOf(const T& item) const {
            CoreAssert(Contains(item), "The PrevsOf an item that is not contained inside the graph was queried!")
            auto res = std::vector<T>();

            auto range = _prevs.equal_range(item);
            std::for_each(range.first, range.second, [&res](auto item) { res.push_back(item.second); });

            return res;
        }

        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all the nodes that are direct or indirect previous nodes of the given one
         */
        [[nodiscard]]
        std::vector<T> AllPrevsOf(const T& item) const {
            return allNodesAfterBFS(item, [this](const T& node) { return PrevsOf(node); });
        }

        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all the nodes that are direct next nodes of the given one
         */
        [[nodiscard]]
        std::vector<T> NextsOf(const T& item) const {
            CoreAssert(Contains(item), "The NextsOf an item that is not contained inside the graph was queried!")
            auto res = std::vector<T>();

            auto range = _nexts.equal_range(item);
            std::for_each(range.first, range.second, [&res](auto item) { res.push_back(item.second); });

            return res;
        }

        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all the nodes that are direct or indirect next nodes of the given one
         */
        [[nodiscard]]
        std::vector<T> AllNextsOf(const T& item) const {
            return allNodesAfterBFS(item, [this](const T& node) { return NextsOf(node); });
        }

        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all prev nodes of the given node, sorted in topological order from the node to the start nodes
         */
        [[nodiscard]]
        std::vector<T> AllNodesFromNodeToStartTopological(const T& item) const {
            auto allPrevs = AllPrevsOf(item);
            return topSort(
                std::vector<T>{item}, // initial node
                [this](const T& node) { return PrevsOf(node); }, // expand function
                [this, &allPrevs, &item](const T& node) {
                    return boolinq::from(NextsOf(node)).where(
                        [&allPrevs, &item](const auto& incoming) {
                            return EqualTo()(incoming, item) || boolinq::from(allPrevs).any(
                                [&incoming](auto prev) {
                                    return EqualTo()(
                                        incoming, prev
                                    );
                                }
                            );
                        }
                    ).toStdVector().size();
                } // incoming edge getter: ignore edges of nodes that are not on the path from the node to the start
            );
        }


        /**
         * @param item A node that must be contained in the graph
         * @return A vector of all next nodes of the given node, sorted in topological order from the node to the end nodes
         */
        [[nodiscard]]
        std::vector<T> AllNodesFromNodeToEndTopological(const T& item) const {
            auto allNexts = AllNextsOf(item);
            return topSort(
                std::vector<T>{item}, // initial node
                [this](const T& node) { return NextsOf(node); }, // expand function
                [this, &allNexts, &item](const T& node) {
                    return boolinq::from(PrevsOf(node)).where(
                        [&allNexts, &item](const auto& incoming) {
                            return EqualTo()(incoming, item) || boolinq::from(allNexts).any(
                                [&incoming](auto next) {
                                    return EqualTo()(
                                        incoming, next
                                    );
                                }
                            );
                        }
                    ).toStdVector().size();
                } // incoming edge getter: ignore edges of nodes that are not on the path from the node to the start
            );
        }


        /**
         * @return All nodes in the graph, sorted topologically, starting at the start nodes and going to the end nodes.
         */
        [[nodiscard]]
        std::vector<T> AllNodesFromStartToEndTopological() const {
            return topSort(
                boolinq::from(_startNodes).toStdVector(), // initial nodes
                [this](const T& node) { return NextsOf(node); }, // expand function
                [this](const T& node) { return PrevsOf(node).size(); } // incoming edge getter
            );
        }

        /**
         * @return All nodes in the graph, sorted topologically, starting at the end nodes and going to the start nodes.
         */
        [[nodiscard]]
        std::vector<T> AllNodesFromEndToStartTopological() const {
            return topSort(
                boolinq::from(_endNodes).toStdVector(), // initial nodes
                [this](const T& node) { return PrevsOf(node); }, // expand function
                [this](const T& node) { return NextsOf(node).size(); } // incoming edge getter
            );
        }

        ///@}

        /**
         * @name Misc
         */
        ///@{

        /**
         * @param item A node that must be contained in the graph
         * @return The length of the longest path for the given node to a start node
         */
        int MaxDistanceFromStart(const T& item) const {
            CoreAssert(Contains(item),
                "The MaxDistanceFromStart of an item that is not contained inside the graph was queried!")

            if (IsStart(item)) return 0;
            else
                return 1 + MaxDistanceFromStart(
                    boolinq::from(PrevsOf(item)).max([this](const auto& prev) { return MaxDistanceFromStart(prev); }));
        }

        /**
         * @param item A node that must be contained in the graph
         * @return The length of the longest path for the given node to an end node
         */
        int MaxDistanceFromEnd(const T& item) const {
            CoreAssert(Contains(item),
                "The MaxDistanceFromEnd of an item that is not contained inside the graph was queried!")

            if (IsEnd(item)) return 0;
            else
                return 1 + MaxDistanceFromEnd(
                    boolinq::from(NextsOf(item)).max([this](const auto& prev) { return MaxDistanceFromEnd(prev); }));
        }

        ///@}

    private:

        [[nodiscard]]
        std::vector<T> topSort(
            std::vector<T> initial, const std::function<std::vector<T>(const T&)> expand,
            const std::function<size_t(const T&)> incomingEdgeGetter
        ) const {
            auto res = std::vector<T>();
            auto remainingEdges = std::unordered_map<T, size_t, Hasher, EqualTo>();
            auto toVisit = std::queue<T>();

            for (auto& item : initial)
                toVisit.push(item);

            while (!toVisit.empty()) {
                auto current = toVisit.front();
                toVisit.pop();

                res.push_back(current);

                for (auto& after : expand(current)) {
                    if (remainingEdges.count(after) == 0) {
                        auto incomingEdges = incomingEdgeGetter(after);
                        CoreAssert(incomingEdges > 0,
                            "An expanded node did not have any incoming edges. This indicates that expand and incoming edge getter are not symmetrical!")
                        if (incomingEdges == 1)
                            toVisit.push(after);
                        else
                            remainingEdges[after] =  incomingEdges - 1;
                    } else {
                        auto remaining = remainingEdges.at(after);
                        CoreAssert(remaining > 0,
                            "An expanded node did not have any remaining edges when re-visited. This indicates that expand and incoming edge getter are not symmetrical!")
                        if (remaining == 1)
                            toVisit.push(after);
                        else
                            remainingEdges[after] = remaining - 1;
                    }
                }
            }

            return res;
        }

        static std::vector<T> allNodesAfterBFS(const T& item, const std::function<std::vector<T>(const T&)> expand) {
            std::vector<T> res{};
            ValueSet discovered{};

            auto toVisit = std::queue<T>();
            toVisit.push(item);

            while (!toVisit.empty()) {
                auto current = toVisit.front();
                toVisit.pop();

                for (auto& after : expand(current)) {
                    if (discovered.count(after) == 0) {
                        discovered.insert(after);
                        res.push_back(after);
                        toVisit.push(after);
                    }
                }
            }

            return res;
        }

        static void eraseKeyValuePair(DependencyMultiMap& multiMap, T key, T value) {
            auto range = multiMap.equal_range(key);
            auto toRemove = std::find_if(
                range.first, range.second, [&key, &value](auto rangeItem) {
                    return EqualTo()(rangeItem.first, key) && EqualTo()(rangeItem.second, value);
                }
            );
            if (toRemove != multiMap.end())
                multiMap.erase(toRemove);
        }

        ValueSet _nodes{};

        ValueSet _startNodes{};
        ValueSet _endNodes{};

        DependencyMultiMap _prevs{};
        DependencyMultiMap _nexts{};
    };
}
