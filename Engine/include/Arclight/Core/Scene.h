#pragma once

#include <Arclight/Core/Node.h>

#include <list>

namespace Arclight {

class Scene
    : public Node {
public:
    Scene();

    template<typename ...Nodes>
    inline Scene(std::shared_ptr<Nodes>... nodes) {
        Scene();

        (AddChild(std::move(nodes)), ...);
    }

    void Start() override;
    void Tick() override;
};

} // namespace Arclight