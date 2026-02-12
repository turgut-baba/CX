#ifndef MLIR_EMITTER_H
#define MLIR_EMITTER_H

#include <memory>

namespace MLIR {

    class Pass;

    class Emitter {
    public:
        Emitter() {};
        void Emit();

    private:
        std::unique_ptr<Pass> createShapeInferencePass();

        std::unique_ptr<mlir::Pass> createLowerToAffinePass();

        std::unique_ptr<mlir::Pass> createLowerToLLVMPass();
    }
}

#endif