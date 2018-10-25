#include "BoundingBox.hpp"
#include "read_chunk.hpp"

#include <string>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

// BoundingBox
// constructor called by BoundingBoxBuffer
BoundingBox::BoundingBox(std::vector< glm::vec2 > &&offsets_) {
    assert(offsets_.size() == 4);

    this->offsets[0] = offsets_[0];
    this->offsets[1] = offsets_[1];
    this->offsets[2] = offsets_[2];
    this->offsets[3] = offsets_[3];

    this->width = this->offsets[1].x - this->offsets[0].x;
    this->thickness = this->offsets[3].y - this->offsets[0].y;
}

void BoundingBox::init_obj_center(const glm::vec2 &obj_center_) {
    // this function can be called only at init stage (so the object is upright)
    assert(this->normal.x == 0.0f);
    assert(this->normal.y == 1.0f);

    this->obj_center = obj_center_;
    this->p0 = this->obj_center + this->offsets[0];

    glm::vec2 p0 = this->obj_center + this->offsets[0];
    glm::vec2 p1 = this->obj_center + this->offsets[1];
    glm::vec2 p3 = this->obj_center + this->offsets[3];
    glm::vec2 bbx_center = glm::vec2((p1.x + p0.x)/2.0f, (p3.y + p0.y)/2.0f);
    this->obj_to_bbx_center_offset = glm::length(bbx_center - this->obj_center);
    if (bbx_center.y < this->obj_center.y) this->obj_to_bbx_center_offset *= -1.0f;
}

void BoundingBox::update_origin(const glm::vec2 &obj_center_, bool normal_changed) {
    this->obj_center = obj_center_;
    auto bbx_center_ = this->obj_center + this->obj_to_bbx_center_offset*this->normal;
    float dw = this->width / 2.0f;
    float dt = this->thickness / 2.0f;
    if (normal_changed) {  // updata parallel only when normal is changed
        glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -90.0f * float(M_PI) / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        this->parallel = glm::vec2(rotation * glm::vec4(this->normal, 0.0f, 1.0f));
    }

    this->p0 = bbx_center_ - this->normal * dt - this->parallel * dw;
}

void BoundingBox::update_origin(const glm::vec2 &obj_center_, const glm::vec2 &normal_) {
    this->normal = normal_;
    update_origin(obj_center_, true);
}

std::vector< glm::vec2 > BoundingBox::get_corners() const {
    return std::vector< glm::vec2 > {
        this->p0,
        this->p0 + this->parallel*this->width,
        this->p0 + this->parallel*this->width + this->normal*this->thickness,
        this->p0 + this->normal*this->thickness
    };
}

bool BoundingBox::is_enclosing(const std::shared_ptr< BoundingBox > object_bbx) const {
    std::vector< glm::vec2 > object_corners = object_bbx->get_corners();
    for (auto &corner : object_corners) {
        // project corner onto parallel and normal
        float parallel_proj = glm::dot(corner - this->p0, this->parallel);
        float normal_proj = glm::dot(corner - this->p0, this->normal);

        // check projected lengh
        if (parallel_proj < 0.0f || parallel_proj > this->width) {
            return false;
        }
        if (normal_proj < 0.0f || normal_proj > this->thickness) {
            return false;
        }
    }

    return true;
}

// BoundingBoxBuffer
BoundingBoxBuffer::BoundingBoxBuffer(std::string const &filename) {
	std::ifstream file(filename, std::ios::binary);

    // read bbx vertices and create BoundingBox objects
    std::vector< glm::vec2 > vertices;
    read_chunk(file, "bbx0", &vertices);
    uint32_t total = vertices.size();

    // read names
	std::vector< char > strings;
	read_chunk(file, "str0", &strings);

    // read index chunk, add to bbxes
    struct IndexEntry {
        uint32_t name_begin, name_end;
        uint32_t vertex_begin, vertex_end;
    };
    static_assert(sizeof(IndexEntry) == 16, "Index entry should be packed");

    std::vector< IndexEntry > index;
    read_chunk(file, "idx0", &index);

    for (auto const &entry : index) {
        if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings.size())) {
            throw std::runtime_error("index entry has out-of-range name begin/end");
        }
        if (!(entry.vertex_begin <= entry.vertex_end && entry.vertex_end <= total)) {
            throw std::runtime_error("index entry has out-of-range vertex start/count");
        }

        std::string name(&strings[0] + entry.name_begin, &strings[0] + entry.name_end);
        std::shared_ptr< BoundingBox > bbx = std::make_shared< BoundingBox >(
            std::vector< glm::vec2 > (vertices.begin() + entry.vertex_begin, vertices.begin() + entry.vertex_end)
        );

        bool inserted = bbxes.insert(std::make_pair(name, bbx)).second;
        if (!inserted) {
            std::cerr << "WARNING: mesh name '" + name + "' in filename '" + filename + "' collides with existing mesh." << std::endl;
        }
    }

	if (file.peek() != EOF) {
		std::cerr << "WARNING: trailing data in mesh file '" << filename << "'" << std::endl;
	}
}

std::shared_ptr< BoundingBox > BoundingBoxBuffer::lookup(std::string const &name) const {
	auto f = bbxes.find(name);
	if (f == bbxes.end()) {
		throw std::runtime_error("Looking up mesh '" + name + "' that doesn't exist.");
	}
    return std::make_shared< BoundingBox >(*(f->second));  // make a copy
}