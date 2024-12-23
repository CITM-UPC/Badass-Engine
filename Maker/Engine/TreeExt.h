#pragma once

#include <list>
#include "readOnlyView.h"
#include "Log.h"

template <class T>
class TreeExt {

private:
	T* _parent = nullptr;
	std::list<T> _children;
	int id; // Unique identifier for each object

public:
	TreeExt(int id) : id(id) {}
	auto& parent() const { return *_parent; }
	auto children() const { return readOnlyListView<T>(_children); }

	auto& root() const { return _parent ? _parent->root() : *this; }
	bool isRoot() const { return !_parent; }

	void removeChild(const T& child) { _children.remove(child);}
	auto& getChildren() { return _children; }

	template <typename ...Args>
	auto& emplaceChild(Args&&... args) {
		_children.emplace_back(std::forward<Args>(args)...);
		_children.back()._parent = static_cast<T*>(this);
		return _children.back();
	}

	//template <typename ...Args>
    auto& setParent(T& newParent = *static_cast<T*>(nullptr)) {
        if (_parent == &newParent) {
            return newParent;
        }
        else if (&newParent == nullptr) {
            // If the new parent is null, simply remove the object from the old parent's children list
            if (_parent) {
                auto it = std::find_if(_parent->_children.begin(), _parent->_children.end(),
                    [this](const T& child) { return child.id == this->id; });

                if (it != _parent->_children.end()) {
                    _parent->_children.erase(it);
                }
            }
            _parent = nullptr;
            return newParent;
        }
        else if (_parent) {
            // Find the position of the current object in the old parent's children list
            auto it = std::find_if(_parent->_children.begin(), _parent->_children.end(),
                [this](const T& child) { return child.id == this->id; });

            if (it != _parent->_children.end()) {
                // Transfer the current object from the old parent's children list to the new parent's children list
                newParent._children.splice(newParent._children.end(), _parent->_children, it);
            }
        }
        else {
            // If there is no current parent, simply add the object to the new parent's children list
            newParent._children.push_back(*static_cast<T*>(this));
        }

        // Set the new parent
        _parent = &newParent;

        return newParent;
    }


	

	

};

