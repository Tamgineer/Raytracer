#pragma once

#include "hittable.h"
#include "pdf.h"
#include "texture.h"

#include <algorithm>

class scatter_record {
  public:
    color attenuation;
    std::shared_ptr<pdf> pdf_ptr;
    bool skip_pdf;
    ray skip_pdf_ray;
};

class material {
    public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const {
      return false;
    }

    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
        return color(0,0,0);
    }

    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
        return 0;
    } 

    virtual bool rgb(const ray& r_in, const hit_record& rec, color& color) const {
      return false;
    }

    virtual bool scatter_normal(const ray& r_in, const hit_record& rec, color& normals, ray& scattered) const {
      return false;
    }
};

class lambertian : public material {
  public:
    lambertian(const color& albedo) : tex(std::make_shared<solid_color>(albedo)) {}
    lambertian(std::shared_ptr<texture> tex) : tex(tex) {}
 
    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = std::make_shared<cosine_pdf>(rec.normal);
        srec.skip_pdf = false;
        return true;
    }
    
    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override {
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        return cos_theta < 0 ? 0 : cos_theta/pi;
      }

    bool scatter_normal(const ray& r_in, const hit_record& rec, color& normals, ray& scattered) const override 
    {
      normals = rec.normal;
      return true;
    }

  private:
    std::shared_ptr<texture> tex;
    color albedo;
};

class metal : public material {
  public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        
        srec.attenuation = albedo;
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());

        return false;
    }


    bool scatter_normal(const ray& r_in, const hit_record& rec, color& normals, ray& scattered) const override 
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected);
        scattered = ray(rec.p, reflected, r_in.time());
        normals = unit_vector(reflected);
        return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}
 
    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        srec.attenuation = color(1.0, 1.0, 1.0);
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;  
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

        srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());

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

class diffuse_light : public material {
  public:
    diffuse_light(std::shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(std::make_shared<solid_color>(emit)) {}

    color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override {
        if (!rec.front_face)
            return color(0,0,0);

        return tex->value(u, v, p);
    }

    bool scatter_normal(const ray& r_in, const hit_record& rec, color& normals, ray& scattered) const override 
    {
      normals = rec.normal;
      return true;
    }

  private:
    std::shared_ptr<texture> tex;
};

class mix : public material {
  public:
  mix(std::shared_ptr<material> a, std::shared_ptr<material> b, double val) : v(std::clamp(val, 0.0, 1.0)), matA(a), matB(b){}
 
  bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
    if(random_double() > v){
      return matA->scatter(r_in, rec, srec);
    } else {
      return matB->scatter(r_in, rec, srec);
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
 
  bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
    if(random_double() > tex->alpha(rec.u, rec.v, rec.p)){
      return matA->scatter(r_in, rec, srec);
    } else {
      return matB->scatter(r_in, rec, srec);
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
  
  bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
    //attenuation = color(1.0, 1.0, 1.0);

    //scattered = ray(rec.p, r_in.direction(), r_in.time());

    return true;
  }
};

class unlit : public material {
  public:
  unlit(const color& albedo) : tex(std::make_shared<solid_color>(albedo)) {}
  unlit(std::shared_ptr<texture> tex) : tex(tex) {}

  //bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    //auto scatter_direction = rec.normal;

    //scattered = ray(rec.p, -scatter_direction);
    //attenuation = albedo;
    //return false; 
  //}

  bool rgb(const ray& r_in, const hit_record& rec, color& color) const override { 
    color = tex->value(rec.u, rec.v, rec.p);
    return true;
  }

  private:
  color albedo;
  std::shared_ptr<texture> tex;
};

class normalMat : public material {
  public:
  normalMat(){}
  bool rgb(const ray& r_in, const hit_record& rec, color& color) const override { 
    vec3 colour = vec3(std::abs(rec.normal.x()), std::abs(rec.normal.y()), std::abs(rec.normal.z()));
    color = colour;
    return true;
  }
};

class depthMat : public material {
  public:
  depthMat(){}
  bool rgb(const ray& r_in, const hit_record& rec, color& color) const override {
    //double z = (2.0 * near * far) / (far + near - (z * 2.0 - 1.0) * (far - near));
    double z = slope * (rec.z - near);
    //z = (1/z - 1/near)/(1/far - 1/near); //attempt non linear depth later on
    //square to make non linear result
    z = std::fabs(z*z);
    color = vec3(z);
    return true;
  }

  private:
  double near = 0.1;
  double far = 2000;
  double slope = 1 / (near - far);

};

class fakeShadows : public material {
  public:
  //arbitrary point in space if none is defined;
  fakeShadows(const color& albedo) : albedo(albedo), lightPos(point3(100, 100, 100)){}
  fakeShadows(const color& albedo, const point3& lightPos) : albedo(albedo), lightPos(lightPos){}
  
  bool rgb(const ray& r_in, const hit_record& rec, color& color) const override {
    color = dot(unit_vector(r_in.direction()), unit_vector(lightPos - rec.p)) > 0.7 ? albedo : albedo * 0.5;
    return true;
  }

  private:
  color albedo;
  point3 lightPos;
};