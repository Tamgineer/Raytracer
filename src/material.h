#pragma once

#include "hittable.h"
#include "texture.h"

#include <algorithm>

class material {
    public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
      return false;
    }

    virtual bool rgb(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
      return false;
    }
};

class lambertian : public material {
  public:
    lambertian(const color& albedo) : tex(std::make_shared<solid_color>(albedo)) {}
    lambertian(std::shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_unit_vector();
        
        // Catch degenerate scatter direction
        if (scatter_direction.near_zero()) {
            scatter_direction = rec.normal;
        }
        
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

  private:
    std::shared_ptr<texture> tex;
    color albedo;
};

class metal : public material {
  public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected, r_in.time());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction, r_in.time());

        return true;
    }

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double reflectance(double cosine, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class mix : public material {
  public:
  mix(std::shared_ptr<material> a, std::shared_ptr<material> b, double val) : v(std::clamp(val, 0.0, 1.0)), matA(a), matB(b){}

  bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override { 
    if(random_double() > v){
      return matA->scatter(r_in, rec, attenuation, scattered);
    } else {
      return matB->scatter(r_in, rec, attenuation, scattered);
    }
  }

  private:
  double v;
  std::shared_ptr<material> matA;
  std::shared_ptr<material> matB;
};

//there's probably a way more elegant way to plug in textures whilst using the same material
//for the time being this material only accepts image textures, if we want to include noise later on, figure
//out how is alpha determined
class textureMix : public material {
  public:
  textureMix(std::shared_ptr<material> a, std::shared_ptr<material> b, std::shared_ptr<image_texture> texture) : tex(texture), matA(a), matB(b){}

  bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override { 
    if(random_double() > tex->alpha(rec.u, rec.v, rec.p)){
      return matA->scatter(r_in, rec, attenuation, scattered);
    } else {
      return matB->scatter(r_in, rec, attenuation, scattered);
    }
  }

  private:
  std::shared_ptr<material> matA;
  std::shared_ptr<material> matB;
  std::shared_ptr<image_texture> tex;
};

//debug materials
class transparent : public material {
  public:
  bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    attenuation = color(1.0, 1.0, 1.0);

    scattered = ray(rec.p, r_in.direction(), r_in.time());

    return true;
  }
};

class unlit : public material {
  public:
  unlit(const color& albedo) : albedo(albedo){}

  //bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    //auto scatter_direction = rec.normal;

    //scattered = ray(rec.p, -scatter_direction);
    //attenuation = albedo;
    //return false; 
  //}

  bool rgb(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override { 
    attenuation = albedo;
    return true;
  }

  private:
  color albedo;
};

class normalMat : public material {
  public:
  normalMat(){}
  bool rgb(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override { 
    attenuation = rec.normal;
    return true;
  }
};

class depthMat : public material {
  public:
  depthMat(){}
  bool rgb(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    //double z = (2.0 * near * far) / (far + near - (z * 2.0 - 1.0) * (far - near));
    double z = slope * (rec.z - near);
    //z = (1/z - 1/near)/(1/far - 1/near); //attempt non linear depth later on
    z = std::fabs(z);
    attenuation = vec3(z);
    return true;
  }

  private:
  double near = 0.1;
  double far = 100;
  double slope = (1 - 0) / (near - far);

};