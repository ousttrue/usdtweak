///
/// This file should be included in Commands.cpp to be compiled with it.
/// IT SHOULD NOT BE COMPILED SEPARATELY
///

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/namespaceEdit.h>
#include <pxr/usd/sdf/valueTypeName.h>
#include <pxr/usd/sdf/copyUtils.h>
#include "ProxyHelpers.h"

PXR_NAMESPACE_USING_DIRECTIVE

struct PrimNew : public SdfLayerCommand {

    // Create a root prim
    PrimNew(SdfLayerRefPtr layer, std::string primName) : _primSpec(), _layer(layer), _primName(std::move(primName)) {}

    // Create a child prim
    PrimNew(SdfPrimSpecHandle primSpec, std::string primName)
        : _primSpec(std::move(primSpec)), _layer(), _primName(std::move(primName)) {}

    ~PrimNew() override {}

    bool DoIt() override {
        if (!_layer && !_primSpec)
            return false;
        if (_layer) {
            SdfUndoRecorder recorder(_undoCommands, _layer);
            _newPrimSpec = SdfPrimSpec::New(_layer, _primName, SdfSpecifier::SdfSpecifierDef);
            _layer->InsertRootPrim(_newPrimSpec);
            return true;
        } else {
            SdfUndoRecorder recorder(_undoCommands, _primSpec->GetLayer());
            _newPrimSpec = SdfPrimSpec::New(_primSpec, _primName, SdfSpecifier::SdfSpecifierDef);
            return true;
        }
    }

    SdfPrimSpecHandle _newPrimSpec;
    SdfPrimSpecHandle _primSpec;
    SdfLayerRefPtr _layer;
    std::string _primName;
};

struct PrimRemove : public SdfLayerCommand {

    PrimRemove(SdfPrimSpecHandle primSpec) : _primSpec(std::move(primSpec)) {}

    ~PrimRemove() override {}

    bool DoIt() override {
        if (!_primSpec)
            return false;
        auto layer = _primSpec->GetLayer();
        SdfUndoRecorder recorder(_undoCommands, layer);
        if (_primSpec->GetNameParent()) {
            // Case where the prim is a variant
            // I am not 100% sure this it the way to do it
            if (_primSpec->GetPath().IsPrimVariantSelectionPath()) {
                auto selection = _primSpec->GetPath().GetVariantSelection();
                TF_FOR_ALL(variantSet, _primSpec->GetNameParent()->GetVariantSets()) {
                    if (variantSet->first == selection.first) {
                        SdfVariantSetSpecHandle variantSetSpec = variantSet->second;
                        SdfVariantSpecHandle variantSpec = variantSetSpec->GetVariants().get(selection.second);
                        if (variantSpec) {
                            variantSetSpec->RemoveVariant(variantSpec);
                            return true;
                        }
                    }
                }
                return false;
            } else {
                return _primSpec->GetNameParent()->RemoveNameChild(_primSpec);
            }
        } else {
            layer->RemoveRootPrim(_primSpec);
            return true;
        }
    }

    SdfPrimSpecHandle _primSpec;
};

template <typename ItemType> struct PrimCreateListEditorOperation : SdfLayerCommand {
    PrimCreateListEditorOperation(SdfPrimSpecHandle primSpec, int operation, typename ItemType::value_type item)
        : _primSpec(primSpec), _operation(operation), _item(std::move(item)) {}
    ~PrimCreateListEditorOperation() override {}

    bool DoIt() override {
        if (_primSpec) {
            SdfUndoRecorder recorder(_undoCommands, _primSpec->GetLayer());
            CreateListEditorOperation(GetListEditor(), _operation, _item);
            return true;
        }
        return false;
    }

    // Forced to inherit as the Specialize and Inherit arcs have the same type
    virtual SdfListEditorProxy<ItemType> GetListEditor() = 0;

    SdfPrimSpecHandle _primSpec;
    int _operation;
    typename ItemType::value_type _item;
};

struct PrimCreateReference : public PrimCreateListEditorOperation<SdfReferenceTypePolicy> {
    using PrimCreateListEditorOperation<SdfReferenceTypePolicy>::PrimCreateListEditorOperation;
    SdfReferencesProxy GetListEditor() override { return _primSpec->GetReferenceList(); }
};

struct PrimCreatePayload : public PrimCreateListEditorOperation<SdfPayloadTypePolicy> {
    using PrimCreateListEditorOperation<SdfPayloadTypePolicy>::PrimCreateListEditorOperation;
    SdfPayloadsProxy GetListEditor() override { return _primSpec->GetPayloadList(); }
};

struct PrimCreateInherit : public PrimCreateListEditorOperation<SdfPathKeyPolicy> {
    using PrimCreateListEditorOperation<SdfPathKeyPolicy>::PrimCreateListEditorOperation;
    SdfInheritsProxy GetListEditor() override { return _primSpec->GetInheritPathList(); }
};

struct PrimCreateSpecialize : public PrimCreateListEditorOperation<SdfPathKeyPolicy> {
    using PrimCreateListEditorOperation<SdfPathKeyPolicy>::PrimCreateListEditorOperation;
    SdfSpecializesProxy GetListEditor() override { return _primSpec->GetSpecializesList(); }
};

struct PrimReparent : public SdfLayerCommand {
    PrimReparent(SdfLayerHandle layer, SdfPath source, SdfPath destination)
        : _layer(std::move(layer)), _source(source), _destination(destination) {}

    ~PrimReparent() override {}

    bool DoIt() override {
        if (!_layer)
            return false;
        SdfUndoRecorder recorder(_undoCommands, _layer);
        SdfNamespaceEdit reparentEdit = SdfNamespaceEdit::Reparent(_source, _destination, 0);
        SdfBatchNamespaceEdit batchEdit;
        batchEdit.Add(reparentEdit);
        if (_layer->CanApply(batchEdit)) {
            _layer->Apply(batchEdit);
            return true;
        }
        return false;
    }

    SdfLayerHandle _layer;
    SdfPath _source;
    SdfPath _destination;
};

struct PrimCreateAttribute : public SdfLayerCommand {

    PrimCreateAttribute(SdfPrimSpecHandle owner, std::string name, SdfValueTypeName typeName,
                       SdfVariability variability = SdfVariabilityVarying, bool custom = false)
        : _owner(std::move(owner)), _name(std::move(name)), _typeName(std::move(typeName)), _variability(variability),
          _custom(custom) {}

    ~PrimCreateAttribute() override {}

    bool DoIt() override {
        if (!_owner)
            return false;
        auto layer = _owner->GetLayer();
        SdfUndoRecorder recorder(_undoCommands, layer);
        if (SdfAttributeSpecHandle attribute = SdfAttributeSpec::New(_owner, _name, _typeName, _variability, _custom)) {
            // Default value for now
            auto defaultValue = _typeName.GetDefaultValue();
            attribute->SetDefaultValue(defaultValue);
            return true;
        }
        return false;
    }
    //
    SdfPrimSpecHandle _owner;
    std::string _name;
    SdfValueTypeName _typeName = SdfValueTypeNames->Float;
    SdfVariability _variability = SdfVariabilityVarying;
    bool _custom = false;
};


struct PrimCreateRelationship : public SdfLayerCommand {

    PrimCreateRelationship(SdfPrimSpecHandle owner, std::string name, SdfVariability variability, bool custom, int operation,
                           std::string targetPath)
        : _owner(std::move(owner)), _name(std::move(name)), _variability(variability), _custom(custom), _operation(operation),
          _targetPath(targetPath) {}

    ~PrimCreateRelationship() override {}

    bool DoIt() override {
        if (!_owner)
            return false;
        auto layer = _owner->GetLayer();
        SdfUndoRecorder recorder(_undoCommands, layer);
        if (SdfRelationshipSpecHandle relationship = SdfRelationshipSpec::New(_owner, _name, _custom, _variability)) {
            CreateListEditorOperation(relationship->GetTargetPathList(), _operation, SdfPath(_targetPath));
            return true;
        }
        return false;
    }
    //
    SdfPrimSpecHandle _owner;
    std::string _name;
    SdfVariability _variability = SdfVariabilityVarying;
    bool _custom = false;
    int _operation = 0;
    std::string _targetPath;
};

struct PrimReorder : public SdfLayerCommand {
    PrimReorder(SdfPrimSpecHandle prim, bool up) : _prim(std::move(prim)), _up(up) {}
    ~PrimReorder() override {}
    bool DoIt() override {
        if (!_prim)
            return false;
        auto layer = _prim->GetLayer();

        TfToken name = _prim->GetNameToken();
        // Look for parent .. layer or prim
        // and find the position of the prim in the parent
        int position = -1;
        auto parent = _prim->GetNameParent();
        const auto &nameChildren = parent ? parent->GetNameChildren() : _prim->GetLayer()->GetRootPrims();
        for (int i = 0; i < nameChildren.size(); ++i) {
            if (nameChildren[i]->GetNameToken() == name) {
                position = i;
                break;
            }
        }

        if (position == -1)
            return false;
        position = _up ? position - 1 : position + 2;
        if (position < 0 || position > nameChildren.size())
            return false;

        SdfUndoRecorder recorder(_undoCommands, layer);
        SdfNamespaceEdit reorderEdit = SdfNamespaceEdit::Reorder(_prim->GetPath(), position);
        SdfBatchNamespaceEdit batchEdit;
        batchEdit.Add(reorderEdit);
        if (layer->CanApply(batchEdit)) {
            layer->Apply(batchEdit);
            return true;
        }
        return false;
    }

    bool _up = true;
    SdfPrimSpecHandle _prim;
};

struct PrimDuplicate : public SdfLayerCommand {
    PrimDuplicate(SdfPrimSpecHandle prim, std::string &newName) : _prim(std::move(prim)), _newName(newName){};
    ~PrimDuplicate() override {}
    bool DoIt() override {
        if (_prim) {
            SdfUndoRecorder recorder(_undoCommands, _prim->GetLayer());
            return (SdfCopySpec(_prim->GetLayer(), _prim->GetPath(), _prim->GetLayer(),
                                _prim->GetPath().ReplaceName(TfToken(_newName))));
        }
        return false;
    }

    std::string _newName;
    SdfPrimSpecHandle _prim;
};

struct CopyPasteCommand : public SdfLayerCommand {
    ~CopyPasteCommand() override {}
    static SdfLayerRefPtr _copyPasteLayer;
};
SdfLayerRefPtr CopyPasteCommand::_copyPasteLayer(SdfLayer::CreateAnonymous("CopyPasteBuffer"));

struct PrimCopy : public CopyPasteCommand {
    PrimCopy(SdfPrimSpecHandle prim) : _prim(prim){};
    ~PrimCopy() override {}
    bool DoIt() override {
        if (_prim && _copyPasteLayer) {
            SdfUndoRecorder recorder(_undoCommands, _copyPasteLayer);
            // Ditch root prim
            auto defaultPrimToken = _copyPasteLayer->GetDefaultPrim();
            if (defaultPrimToken != TfToken()) {
                auto defaultPrim = _copyPasteLayer->GetPrimAtPath(SdfPath::AbsoluteRootPath().AppendChild(defaultPrimToken));
                defaultPrim->GetLayer()->RemoveRootPrim(defaultPrim);
            }
            // Copy
            const bool copyOk = SdfCopySpec(_prim->GetLayer(), _prim->GetPath(), _copyPasteLayer,
                                            SdfPath::AbsoluteRootPath().AppendChild(_prim->GetNameToken()));
            if (copyOk) {
                _copyPasteLayer->SetDefaultPrim(_prim->GetNameToken());
            }
            return copyOk;
        }
        return false;
    }
    SdfPrimSpecHandle _prim;
};

struct PrimPaste : public CopyPasteCommand {
    PrimPaste(SdfPrimSpecHandle prim) : _prim(prim){};
    ~PrimPaste() override {}
    bool DoIt() override {
        if (_prim && _copyPasteLayer) {
            SdfUndoRecorder recorder(_undoCommands, _prim->GetLayer());
            return SdfCopySpec(_copyPasteLayer, SdfPath::AbsoluteRootPath().AppendChild(_copyPasteLayer->GetDefaultPrim()),
                               _prim->GetLayer(), _prim->GetPath().AppendChild(_copyPasteLayer->GetDefaultPrim()));
        }
        return false;
    }
    SdfPrimSpecHandle _prim;
};

/// TODO: how to avoid having to write the argument list ? it's the same as the constructor arguments
template void ExecuteAfterDraw<PrimNew>(SdfLayerRefPtr layer, std::string newName);
template void ExecuteAfterDraw<PrimNew>(SdfPrimSpecHandle primSpec, std::string newName);
template void ExecuteAfterDraw<PrimRemove>(SdfPrimSpecHandle primSpec);
template void ExecuteAfterDraw<PrimReparent>(SdfLayerHandle layer, SdfPath source, SdfPath destination);
template void ExecuteAfterDraw<PrimCreateReference>(SdfPrimSpecHandle primSpec, int operation, SdfReference reference);
template void ExecuteAfterDraw<PrimCreatePayload>(SdfPrimSpecHandle primSpec, int operation, SdfPayload payload);
template void ExecuteAfterDraw<PrimCreateInherit>(SdfPrimSpecHandle primSpec, int operation, SdfPath inherit);
template void ExecuteAfterDraw<PrimCreateSpecialize>(SdfPrimSpecHandle primSpec, int operation, SdfPath specialize);
template void ExecuteAfterDraw<PrimCreateAttribute>(SdfPrimSpecHandle owner, std::string name, SdfValueTypeName typeName,
                                                    SdfVariability variability, bool custom);
template void ExecuteAfterDraw<PrimCreateRelationship>(SdfPrimSpecHandle owner, std::string name, SdfVariability variability,
                                                       bool custom, int operation, std::string targetPath);
template void ExecuteAfterDraw<PrimReorder>(SdfPrimSpecHandle owner, bool up);
template void ExecuteAfterDraw<PrimDuplicate>(SdfPrimSpecHandle prim, std::string newName);
template void ExecuteAfterDraw<PrimCopy>(SdfPrimSpecHandle prim);
template void ExecuteAfterDraw<PrimPaste>(SdfPrimSpecHandle prim);