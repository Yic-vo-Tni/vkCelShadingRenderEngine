//
// Created by lenovo on 6/4/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENT_H
#define VKCELSHADINGRENDERER_EVENT_H

namespace yic {

//    class I_Pipeline {
//    public:
//        virtual void recreate() = 0;
//        virtual ~I_Pipeline() = default;
//    };
//
//    template<typename Derived>
//    class Pipeline : public I_Pipeline{
//    public:
//        void recreate() override{
//            static_cast<Derived*>(this)->doRecreate();
//        }
//
//    protected:
//        void baseFunc(){
//            std::cout << " " << "\n";
//        }
//    };
//
//    class GraphicsPipeline : public Pipeline<GraphicsPipeline>{
//    public:
//        void doRecreate(){
//            baseFunc();
//        }
//    };
//
//    class ComputePipeline : public Pipeline<ComputePipeline>{
//    public:
//        void doRecreate(){
//            baseFunc();
//        }
//    };
//
//    void ReCreate(I_Pipeline& p){
//        p.recreate();
//    }
//
//    void x(){
//        GraphicsPipeline g;
//        g.recreate();
//        g.recreate();
//        ReCreate(g);
//    }

} // yic

#endif //VKCELSHADINGRENDERER_EVENT_H
