#include "MLIRGen.h" // TODO: change this to checker/analyzer.

namespace MLIR {
	Generator::Generator()
		:file("test.mlir")
	{
		AddLine("module {");
		ident++;
	}

	Generator::~Generator()
	{
		file.close();
	}

	std::ofstream& Generator::GetFile()
	{
		file << "}";
		ident--;

		return file;
	}

	void Generator::Generate(ArrayAlloc<AST::Statement*>& ASTTree) {
		for (AST::ASTNode* node : ASTTree.vec_) {
			if (node->IsNodeType(AST::NodeType::FUNCTION))
				Gen(static_cast<AST::Function*>(node));
		}

	}

	void Generator::Gen(AST::Function* node)
	{
		AddLine("func.func ", false);
		file << "@" << node->Name() << "(";

/*
		for (auto param : node->Params()) {
			file << param.String();
		}
*/

		AddLine(") -> i32 {");
		ident++;

		for (auto& child : node->Chlidren())
		{
			//if (child != nullptr) Gen(child);
		}

		ident--;
		AddLine("}");
	}

}
