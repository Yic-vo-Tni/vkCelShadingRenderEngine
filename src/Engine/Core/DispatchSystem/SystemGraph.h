//
// Created by lenovo on 7/7/2025.
//

#ifndef VKCELSHADINGRENDERER_SYSTEMGRAPH_H
#define VKCELSHADINGRENDERER_SYSTEMGRAPH_H

namespace hide {

    struct NodeKey {
        std::type_index type;
        vot::string name;

        bool operator==(const NodeKey &rhs) const {
            return type == rhs.type && name == rhs.name;
        }
    };

}

namespace std{

    template<>
    struct hash<hide::NodeKey>{
        size_t operator()(const hide::NodeKey& k) const noexcept{
            return hash<std::type_index>()(k.type) ^ hash<vot::string>()(k.name);
        }
    };
}

namespace hide{
    class ISystem : public std::enable_shared_from_this<ISystem> {
    public:
        virtual ~ISystem() = default;
        virtual auto name() const -> vot::string = 0;
        virtual auto run() -> void = 0;


        virtual auto read() const -> const vot::vector<NodeKey>& = 0;
        virtual auto write() const -> const vot::vector<NodeKey>& = 0;
    };

    struct SystemNode{
        std::shared_ptr<ISystem> system;
        std::weak_ptr<ISystem> parentSystem;

        //
        vot::vector<std::shared_ptr<ISystem>> childSystems;
    };

    class SystemGraph {
    public:
        auto reg_node(const SystemNode& node) -> void{
            auto& sys = *node.system;
            NodeKey key{typeid(sys), node.system->name()};
            assert(!nodeMap.contains(key) && "system already exists!");
            nodeMap[key] = node;
        }

        auto build_dep() -> void{
//            vot::unordered_map<NodeKey, NodeKey> lastWriter;
//
//            for(const auto& [sysKey, node] : nodeMap){
//                for(const auto& res : node.system->read()){
//                    if (lastWriter.contains(res)){
//                        deps[sysKey].insert(lastWriter[res]);
//                    }
//                }
//                for(const auto& res : node.system->write()){
//                    if (lastWriter.contains(res)){
//                        deps[sysKey].insert(lastWriter[res]);
//                    }
//                    lastWriter[res] = sysKey;
//                }
//            }
        }

        auto run_all() -> void {
            vot::unordered_map<NodeKey, int> indegree;
            for (const auto& [sys, ds] : deps) {
                for (const auto& d : ds) indegree[d]++;
                indegree[sys] += 0;
            }
            vot::queue<NodeKey> q;
            for (const auto& [sys, deg] : indegree)
                if (deg == 0) q.push(sys);

            vot::vector<NodeKey> order;
            while (!q.empty()) {
                auto sys = q.front(); q.pop();
                order.push_back(sys);
                for (const auto& d : deps[sys]) {
                    if (--indegree[d] == 0) q.push(d);
                }
            }

            for (const auto& sys : order) {
                nodeMap[sys].system->run();
            }

        }

    private:
        vot::unordered_map<NodeKey, SystemNode> nodeMap;
        vot::unordered_map<NodeKey, vot::unordered_set<NodeKey>> deps;
    };

} // hide

#endif //VKCELSHADINGRENDERER_SYSTEMGRAPH_H
