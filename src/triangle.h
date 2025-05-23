#pragma once

#include "hittable.h"
#include "hittable_list.h"

class tri : public hittable {
    public:

    tri(const point3& posA, const point3& posB, const point3& posC, const vec3& normA, const vec3& normB, const vec3& normC, std::shared_ptr<material> mat) : 
    pA(posA), pB(posB), pC(posC), nA(normA), nB(normB), nC(normC), mat(mat) {
        edgeAB = pB - pA;
        edgeAC = pC - pA;

        set_bounding_box();

    }

    virtual void set_bounding_box() {
        vec3 bottom = pA;
        vec3 top = pA;

        //bottom
        bottom = bottom.x() > pB.x() ? vec3(pB.x(), bottom.y(), bottom.z()) : bottom;
        bottom = bottom.x() > pC.x() ? vec3(pC.x(), bottom.y(), bottom.z()) : bottom;
        bottom = bottom.y() > pB.y() ? vec3(bottom.x(), pB.y(), bottom.z()) : bottom;
        bottom = bottom.y() > pC.y() ? vec3(bottom.x(), pC.y(), bottom.z()) : bottom;
        bottom = bottom.z() > pB.z() ? vec3(bottom.x(), bottom.y(), pB.z()) : bottom;
        bottom = bottom.z() > pC.z() ? vec3(bottom.x(), bottom.y(), pC.z()) : bottom;

        // top
        top = top.x() < pB.x() ? vec3(pB.x(), top.y(), top.z()) : top;
        top = top.x() < pC.x() ? vec3(pC.x(), top.y(), top.z()) : top;
        top = top.y() < pB.y() ? vec3(top.x(), pB.y(), top.z()) : top;
        top = top.y() < pC.y() ? vec3(top.x(), pC.y(), top.z()) : top;
        top = top.z() > pB.z() ? vec3(top.x(), top.y(), pB.z()) : top;
        top = top.z() > pC.z() ? vec3(top.x(), top.y(), pC.z()) : top;

        std::cout << "posA " << pA.x() << pA.y() << pA.z() << "\n";
        std::cout << "posB " << pB.x() << pB.y() << pB.z() << "\n";
        std::cout << "posC " << pC.x() << pC.y() << pC.z() << "\n";
        std::cout << "top " << top.x() << top.y() << top.z() << "\n";
        std::cout << "bottom" << bottom.x() << bottom.y() << bottom.z() << "\n";
        bbox = aabb(top, bottom);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray &r, interval ray_t, hit_record &rec, const vec3 &camPos) const override {
        vec3 planeNormal = cross(edgeAB, edgeAC);

        double determinant = dot(edgeAB, cross(r.direction(), edgeAC));

        //check if the ray is parallel to the triangle
        if(determinant < 1e-8 || determinant > -1e-8){
            return false;
        }

        double invDeterminant = 1 / determinant;

        double u,v,w;

        vec3 ao = r.origin() - pA;

        u = invDeterminant * dot(ao, cross(r.direction(), edgeAC));

        if ((u < 0 && abs(u) > 1e-8) || (u > 1 && abs(u-1) > 1e-8)){
            return false;
        }

        vec3 ao_cross_ab = cross(ao, edgeAB);
        v = invDeterminant * dot(r.direction(), ao_cross_ab);

        w = u + v + 1;

        if ((v < 0 && abs(v) > 1e-8) || (u + v > 1 && abs(w) > 1e-8)){
            return false;
        }

        //figure out the intersection here
        double t = invDeterminant * dot(edgeAC, ao_cross_ab);

        if(t < 1e-8){
            return false;
        }

        rec.u = 1;
        rec.v = 1;
        rec.t = t;
        rec.p = r.at(t);
        rec.mat = mat;
        rec.set_face_normal(r, planeNormal);
        rec.set_face_depth(r, camPos);

        return true;
    }

    virtual bool is_interior(double a, double b, hit_record& rec) const {
        
        return true;
    }

    private:
    point3 pA, pB, pC;
    vec3 nA, nB, nC;

    vec3 edgeAB;
    vec3 edgeAC;

    std::shared_ptr<material> mat;

    aabb bbox;
};