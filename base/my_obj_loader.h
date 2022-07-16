#pragma once
#include "my_obj_loader_misc.h"

static bool exportFaceGroupToShape(
    shape_t* shape, const std::vector<std::vector<vertex_index> >& faceGroup,
    const std::vector<tag_t>& tags, const int material_id,
    const std::string& name, bool triangulate) {
    if (faceGroup.empty()) {
        return false;
    }

    // Flatten vertices and indices
    for (size_t i = 0; i < faceGroup.size(); i++) {
        const std::vector<vertex_index>& face = faceGroup[i];

        vertex_index i0 = face[0];
        vertex_index i1(-1);
        vertex_index i2 = face[1];

        size_t npolys = face.size();

        if (triangulate) {
            // Polygon -> triangle fan conversion
            for (size_t k = 2; k < npolys; k++) {
                i1 = i2;
                i2 = face[k];

                index_t idx0, idx1, idx2;
                idx0.vertex_index = i0.v_idx;
                idx0.normal_index = i0.vn_idx;
                idx0.texcoord_index = i0.vt_idx;
                idx1.vertex_index = i1.v_idx;
                idx1.normal_index = i1.vn_idx;
                idx1.texcoord_index = i1.vt_idx;
                idx2.vertex_index = i2.v_idx;
                idx2.normal_index = i2.vn_idx;
                idx2.texcoord_index = i2.vt_idx;

                shape->mesh.indices.push_back(idx0);
                shape->mesh.indices.push_back(idx1);
                shape->mesh.indices.push_back(idx2);

                shape->mesh.num_face_vertices.push_back(3);
                //shape->mesh.material_ids.push_back(material_id);
            }
        }
        else {
            for (size_t k = 0; k < npolys; k++) {
                index_t idx;
                idx.vertex_index = face[k].v_idx;
                idx.normal_index = face[k].vn_idx;
                idx.texcoord_index = face[k].vt_idx;
                shape->mesh.indices.push_back(idx);
            }

            shape->mesh.num_face_vertices.push_back(
                static_cast<unsigned char>(npolys));
            //shape->mesh.material_ids.push_back(material_id);  // per face
        }
    }

    shape->name = name;
    shape->mesh.tags = tags;

    return true;
}

bool LoadObj(attrib_t* attrib, std::vector<shape_t>* shapes, std::string* err, const char* filename) {
    bool triangulate = true;
    std::stringstream errss;

    std::ifstream ifs(filename);
    if (!ifs) {
        errss << "Cannot open file [" << filename << "]" << std::endl;
        if (err) {
            (*err) = errss.str();
        }
        return false;
    }
    std::istream* inStream = &ifs;

    std::vector<float> v;
    std::vector<float> vn;
    std::vector<float> vt;
    std::vector<tag_t> tags;
    std::vector<std::vector<vertex_index> > faceGroup;
    std::string name;

    shape_t shape;

    int material = -1;

    std::string linebuf;
    while (inStream->peek() != -1) {
        safeGetline(*inStream, linebuf);

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\n')
                linebuf.erase(linebuf.size() - 1);
        }
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\r')
                linebuf.erase(linebuf.size() - 1);
        }

        // Skip if empty line.
        if (linebuf.empty()) {
            continue;
        }

        // Skip leading space.
        const char* token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0') continue;  // empty line

        if (token[0] == '#') continue;  // comment line

        // vertex
        if (token[0] == 'v' && IS_SPACE((token[1]))) {
            token += 2;
            float x, y, z;
            parseReal3(&x, &y, &z, &token);
            v.push_back(x);
            v.push_back(y);
            v.push_back(z);
            continue;
        }

        // normal
        if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
            token += 3;
            float x, y, z;
            parseReal3(&x, &y, &z, &token);
            vn.push_back(x);
            vn.push_back(y);
            vn.push_back(z);
            continue;
        }

        // texcoord
        if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
            token += 3;
            float x, y;
            parseReal2(&x, &y, &token);
            vt.push_back(x);
            vt.push_back(y);
            continue;
        }

        // face
        if (token[0] == 'f' && IS_SPACE((token[1]))) {
            token += 2;
            token += strspn(token, " \t");

            std::vector<vertex_index> face;
            face.reserve(3);

            while (!IS_NEW_LINE(token[0])) {
                vertex_index vi = parseTriple(&token, static_cast<int>(v.size() / 3),
                    static_cast<int>(vn.size() / 3),
                    static_cast<int>(vt.size() / 2));
                face.push_back(vi);
                size_t n = strspn(token, " \t\r");
                token += n;
            }

            // replace with emplace_back + std::move on C++11
            faceGroup.push_back(std::vector<vertex_index>());
            faceGroup[faceGroup.size() - 1].swap(face);

            continue;
        }

        // group name
        if (token[0] == 'g' && IS_SPACE((token[1]))) {
            // flush previous face group.
            bool ret = exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
                triangulate);
            if (ret) {
                shapes->push_back(shape);
            }

            shape = shape_t();

            // material = -1;
            faceGroup.clear();

            std::vector<std::string> names;
            names.reserve(2);

            while (!IS_NEW_LINE(token[0])) {
                std::string str = parseString(&token);
                names.push_back(str);
                token += strspn(token, " \t\r");  // skip tag
            }

            assert(names.size() > 0);

            // names[0] must be 'g', so skip the 0th element.
            if (names.size() > 1) {
                name = names[1];
            }
            else {
                name = "";
            }

            continue;
        }

        // Ignore unknown command.
    }

    bool ret = exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
        triangulate);
    // exportFaceGroupToShape return false when `usemtl` is called in the last
    // line.
    // we also add `shape` to `shapes` when `shape.mesh` has already some
    // faces(indices)
    if (ret || shape.mesh.indices.size()) {
        shapes->push_back(shape);
    }
    faceGroup.clear();  // for safety

    if (err) {
        (*err) += errss.str();
    }

    attrib->vertices.swap(v);
    attrib->normals.swap(vn);
    attrib->texcoords.swap(vt);

    return true;
}