//
// Created by lenovo on 7/7/2025.
//

#ifndef VKCELSHADINGRENDERER_SYSTEMGRAPH_H
#define VKCELSHADINGRENDERER_SYSTEMGRAPH_H

namespace hide {

    class ISystem : public std::enable_shared_from_this<ISystem> {
    public:
        virtual ~ISystem() = default;
        virtual auto name() const -> vot::string = 0;
        virtual auto run() -> void = 0;

        virtual auto read() const -> const vot::vector<vot::string>& = 0;
        virtual auto write() const -> const vot::vector<vot::string>& = 0;
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
            assert(!nodeMap.contains(node.system->name()) && "system already exists!");
            nodeMap[node.system->name()] = node;
        }

        auto build_dep() -> void{
            vot::unordered_map<vot::string, vot::string> lastWriter;

            vot::unordered_map<vot::string, vot::unordered_set<vot::string>> deps;

            for(const auto& [sysName, node] : nodeMap){
                for(const auto& res : node.system->read()){
                    if (lastWriter.contains(res)){
                        deps[sysName].insert(lastWriter[res]);
                    }
                }

                for(const auto& res : node.system->write()){
                    if (lastWriter.contains(res)){
                        deps[sysName].insert(lastWriter[res]);
                    }
                    lastWriter[res] = sysName;
                }
            }
        }

    private:
        vot::unordered_map<vot::string, SystemNode> nodeMap;
    };

} // hide

#endif //VKCELSHADINGRENDERER_SYSTEMGRAPH_H
