/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include "EditorModule.h"
#include "ecs/systems/System.h"

namespace modulith::editor{

    class SelectionSystem : public modulith::System {
    public:
        SelectionSystem() : modulith::System("Selection System") {}

        template<class T>
        void HandleImGuiSelection(T selected) {
            if (ImGui::GetIO().KeyCtrl) {
                if (SelectionContains(selected))
                    RemoveFromSelection(selected);
                else AddToSelection(selected);
            } else {
                if (IsMultiselection())
                    SetSelection(selected);
                else if (SelectionContains(selected))
                    ClearSelection();
                else SetSelection(selected);
            }
        }

        template<class T>
        void AddToSelection(T toAdd) {
            if (!SelectionContains(toAdd))
                _currentSelection.push_back(toAdd);
        }

        template<class T>
        void RemoveFromSelection(T toRemove) {
            auto remove = std::find_if(
                _currentSelection.begin(), _currentSelection.end(), [&toRemove](const auto& selection) {
                    return selectionItemEquals(selection, toRemove);
                }
            );

            if (remove != _currentSelection.end())
                _currentSelection.erase(remove);
        }

        void SetSelection(std::any newSelection) {
            _currentSelection = std::vector<std::any>{std::move(newSelection)};
        }

        void ClearSelection() { _currentSelection.clear(); }

        template<class T>
        bool SelectionContains(T item) {
            return boolinq::from(_currentSelection).any(
                [&item](const auto& selection) {
                    return selectionItemEquals(selection, item);
                }
            );
        }

        bool IsMultiselection() { return _currentSelection.size() > 1; }
        size_t Count() { return _currentSelection.size(); }

        std::optional<std::reference_wrapper<const std::type_info>> TryGetSingleType() {
            if (_currentSelection.empty()) return std::nullopt;
            auto firstType = std::reference_wrapper(_currentSelection[0].type());
            for (auto& selected : _currentSelection) {
                if (selected.type() != firstType)
                    return std::nullopt;
            }
            return std::optional(firstType);
        }

        std::vector<std::any> AllOf(std::reference_wrapper<const std::type_info> type) {
            return boolinq::from(_currentSelection)
                .where([type](auto& selection) { return selection.type() == type; })
                .toStdVector();
        }

        template<class T>
        std::optional<T> TryGetSingleItem(){
            auto singleType = TryGetSingleType();
            if(singleType && singleType.value().get() == typeid(T) && Count() == 1){
                return std::any_cast<T>(AllOf(typeid(T))[0]);
            }
            return std::nullopt;
        }

    private:

        template<class T>
        static bool selectionItemEquals(std::any selection, T item) {
            if (selection.type() == typeid(T))
                return std::any_cast<T>(selection) == item;
            return false;
        }

        std::vector<std::any> _currentSelection;
    };
}
