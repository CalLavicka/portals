#pragma once

#include <vector>
#include <array>
#include <map>
#include <glm/glm.hpp>

// BoundingBox Usage
//
// In GameMode.cpp
//     Loading default bounding boxes
//         Load< BoundingBoxBuffer > bbxes(LoadTagDefault, [](){
// 	         return new BoundingBoxBuffer(data_path("vegetables.bbx"));
//         });
//
//     generating a new object (e.g. a Carrot)
//         obj->transform->boundingbox = bbxes->lookup("Carrot");  // copy a default Carrot's bbx from buffer
//         obj->transform->boundingbox->init_obj_center(obj->transform->position);  // set position to this Carrot
//
//     when obj changes position/orientation
//         obj->transform->boundingbox->update_origin();  // or wrap this into a function

struct BoundingBox {
	// object normal is pointing upwards
	//     3 --------- 2
	//     |           |
	//     |     O     |   O: object center
	//     |     B     |   B: bbx center
	//     |           |
	//     |           |
	//     0 --------- 1
    
    BoundingBox() {};
    // BoundingBox(float width_, float thickness_) : width(width_), thickness(thickness_) {};  // this would prob be discarded
    BoundingBox(std::vector< glm::vec2 > &&offsets_);

	float width = 0.0f;      // length(0 -> 1)
	float thickness = 0.0f;  // length(0 -> 3)

	glm::vec2 p0;  // lower-left point of the bbx
	glm::vec2 parallel = glm::vec2(1.0f, 0.0f);  // parallel = normalize(0 -> 1)
    glm::vec2 normal = glm::vec2(0.0f, 1.0f);    // normal   = normalize(0 -> 3)

	glm::vec2 obj_center;  // center of the parent object (not necessarily the center of the bbx)
	float obj_to_bbx_center_offset;  // bbx_center = obj_center + obj_to_bbx_center_offset * normal
	std::array< glm::vec2, 4 > offsets;  // the offset from object center to p0, p1, p2, p3
	                                     // i.e. obj_center + offsets[k] = pk

    // initializer 
	void init_obj_center(const glm::vec2 &obj_center_);

	// call either of these when the object is moving/rotating 
    void update_origin(const glm::vec2 &obj_center_, bool normal_changed = false);
    void update_origin(const glm::vec2 &obj_center_, const glm::vec2 &normal_);

    // just a useful thing
	std::vector< glm::vec2 > get_corners() const ;  // return {p0, p1, p2, p3}

	// return true when object_bbx is fully enveloped by caller's bbx
	bool is_enclosing(const std::shared_ptr< BoundingBox > object_bbx) const ;  
};

// all default BoundingBoxes are stored here
// copy one from here when generating a new object
struct BoundingBoxBuffer {
	// load from .bbx file
	BoundingBoxBuffer(std::string const &filename);

    // copy a new BoundingBox from "bbxes[name]" and return its shared_ptr
	std::shared_ptr< BoundingBox > lookup(std::string const &name) const;

	// internal
	std::map< std::string, std::shared_ptr< BoundingBox > > bbxes;
};