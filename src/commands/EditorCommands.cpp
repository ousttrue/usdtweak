///
/// This file should be included in Commands.cpp to be compiled with it.
/// IT SHOULD NOT BE COMPILED SEPARATELY
///

///
/// Work in progress, this is not used yet
///

#include <string>
#include "commands/Commands.h"
#include "Editor.h"

///
/// Base class for an editor command, contai ns only a pointer of the editor
///
struct EditorCommand : public Command {
    static Editor *_editor;
};
Editor *EditorCommand::_editor = nullptr;

/// The editor commands need a pointer to the data
/// Other client can use this function to pass a handle to their data structure
struct EditorSetDataPointer : public EditorCommand {
    EditorSetDataPointer(Editor *editor) {
        if (!_editor)
            _editor = editor;
    }
    bool DoIt() override { return false; }
};
template void ExecuteAfterDraw<EditorSetDataPointer>(Editor *editor);


struct EditorSelectPrimPath : public EditorCommand {

    EditorSelectPrimPath(SdfPath *selectionObject, SdfPath selectedPath)
        : selectionObject(selectionObject), selectedPath(std::move(selectedPath)) {}

    ~EditorSelectPrimPath() override {}

    // TODO: wip, we want an "Selection" object to be passed around
    // At the moment it is just the pointer to the current selection held by the editor
    bool DoIt() override {
        *selectionObject = selectedPath;
        return true;
    }
    SdfPath *selectionObject;
    SdfPath selectedPath;
};
template void ExecuteAfterDraw<EditorSelectPrimPath>(SdfPath *selectionObject, SdfPath selectedPath);

struct EditorOpenStage : public EditorCommand {

    EditorOpenStage(std::string stagePath) : _stagePath(std::move(stagePath)) {}
    ~EditorOpenStage() override {}

    bool DoIt() override {
        if (_editor) {
            _editor->ImportStage(_stagePath);
        }

        return false;
    }

    std::string _stagePath;
};
template void ExecuteAfterDraw<EditorOpenStage>(std::string stagePath);

struct EditorSetCurrentStage : public EditorCommand {

    EditorSetCurrentStage(SdfLayerHandle layer) : _layer(layer) {}

    bool DoIt() override {
        if (_editor) {
            if (auto stage = _editor->GetStageCache().FindOneMatching(_layer)) {
                _editor->SetCurrentStage(stage);
            }
        }
        return false; // never store this command
    }
    SdfLayerHandle _layer;
};
template void ExecuteAfterDraw<EditorSetCurrentStage>(SdfLayerHandle layer);

struct EditorSetEditTarget : public EditorCommand {

    EditorSetEditTarget(SdfLayerHandle layer) : _layer(layer) {}
    ~EditorSetEditTarget() override {}

    bool DoIt() override {
        if (_editor) {
            // TODO: we should check if the stage is already imported ?
            _editor->SetCurrentEditTarget(_layer);
        }

        return false;
    }
    SdfLayerHandle _layer;
};
template void ExecuteAfterDraw<EditorSetEditTarget>(SdfLayerHandle layer);

// Will set the current layer and the current selected layer prim
struct EditorInspectLayerLocation : public EditorCommand {
    EditorInspectLayerLocation(SdfLayerHandle layer, SdfPath path) : _layer(layer), _path(path) {}
    EditorInspectLayerLocation(SdfLayerRefPtr layer, SdfPath path) : _layer(layer), _path(path) {}
    ~EditorInspectLayerLocation() override {}

    bool DoIt() override {
        if (_editor && _layer) {
            _editor->SetCurrentLayer(_layer);
            _editor->SetSelectedPrimSpec(_path);
            _editor->ShowLayerEditor();
        }

        return false;
    }
    SdfLayerRefPtr _layer;
    SdfPath _path;
};
template void ExecuteAfterDraw<EditorInspectLayerLocation>(SdfLayerHandle layer, SdfPath);
template void ExecuteAfterDraw<EditorInspectLayerLocation>(SdfLayerRefPtr layer, SdfPath);

struct EditorSetCurrentLayer : public EditorCommand {

    EditorSetCurrentLayer(SdfLayerHandle layer) : _layer(layer) {}
    ~EditorSetCurrentLayer() override {}

    bool DoIt() override {
        if (_editor && _layer) {
            _editor->SetCurrentLayer(_layer);
            _editor->ShowLayerEditor();
        }

        return false;
    }
    SdfLayerRefPtr _layer;

};
template void ExecuteAfterDraw<EditorSetCurrentLayer>(SdfLayerHandle layer);


struct EditorOpenLayer : public EditorCommand {

    EditorOpenLayer(std::string layerPath) : _layerPath(std::move(layerPath)) {}
    ~EditorOpenLayer() override {}

    bool DoIt() override {
        if (_editor) {
            _editor->ImportLayer(_layerPath);
        }

        return false;
    }
    std::string _layerPath;
};
template void ExecuteAfterDraw<EditorOpenLayer>(std::string layerPath);


struct EditorSetPreviousLayer : public EditorCommand {

    EditorSetPreviousLayer() {}
    ~EditorSetPreviousLayer() override {}

    bool DoIt() override {
        if (_editor) {
            _editor->SetPreviousLayer();
        }
        return false;
    }
};
template void ExecuteAfterDraw<EditorSetPreviousLayer>();

struct EditorSetNextLayer : public EditorCommand {

    EditorSetNextLayer() {}
    ~EditorSetNextLayer() override {}

    bool DoIt() override {
        if (_editor) {
            _editor->SetNextLayer();
        }
        return false;
    }
};
template void ExecuteAfterDraw<EditorSetNextLayer>();

