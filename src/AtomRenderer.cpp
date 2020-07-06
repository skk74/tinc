#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "tinc/AtomRenderer.hpp"

using namespace tinc;

using namespace al;

void InstancingMesh::init(const std::string &vert_str,
                          const std::string &frag_str, GLuint attrib_loc,
                          GLint attrib_num_elems, GLenum attrib_type) {
  shader.compile(vert_str, frag_str);
  buffer.bufferType(GL_ARRAY_BUFFER);
  buffer.usage(GL_DYNAMIC_DRAW); // assumes buffer will change every frame
  // and will be used for drawing
  buffer.create();

  auto &v = mesh.vao();
  v.bind();
  v.enableAttrib(attrib_loc);
  // for normalizing, this code only considers GL_FLOAT AND GL_UNSIGNED_BYTE,
  // (does not normalize floats and normalizes unsigned bytes)
  v.attribPointer(attrib_loc, buffer, attrib_num_elems, attrib_type,
                  (attrib_type == GL_FLOAT) ? GL_FALSE : GL_TRUE, // normalize?
                  0,                                              // stride
                  0);                                             // offset
  glVertexAttribDivisor(attrib_loc, 1); // step attribute once per instance
}

void InstancingMesh::attrib_data(size_t size, const void *data, size_t count) {
  buffer.bind();
  buffer.data(size, data);
  num_instances = count;
}

void InstancingMesh::draw() {
  mesh.vao().bind();
  if (mesh.indices().size()) {
    mesh.indexBuffer().bind();
    glDrawElementsInstanced(mesh.vaoWrapper->GLPrimMode, mesh.indices().size(),
                            GL_UNSIGNED_INT, 0, num_instances);
  } else {
    glDrawArraysInstanced(mesh.vaoWrapper->GLPrimMode, 0,
                          mesh.vertices().size(), num_instances);
  }
}

void AtomRenderer::init() {
  // Define mesh for instance drawing
  addSphere(instancingMesh.mesh, 1, 12, 6);
  instancingMesh.mesh.update();

  std::string funcMarker = "//[[FUNCTION:is_highlighted(vec3 point)]]";

  std::size_t pos = instancing_vert.find(funcMarker);
  if (pos != std::string::npos) {
    instancing_vert.replace(pos, funcMarker.length(), is_highlighted_func());
  }
  instancingMesh.init(instancing_vert, instancing_frag,
                      1,         // location
                      4,         // num elements
                      GL_FLOAT); // type

  instancing_shader.compile(instancing_vert, instancing_frag);

  mMarkerScale = 0.01f;
}

void AtomRenderer::setDataBoundaries(al::BoundingBoxData &b) {
  dataBoundary = b;
}

void AtomRenderer::draw(al::Graphics &g, float scale,
                        std::map<std::string, AtomData> &mAtomData,
                        std::vector<float> &mAligned4fData) {
  g.polygonFill();
  // now draw data with custom shaderg.shader(instancing_mesh0.shader);
  g.shader(instancingMesh.shader);
  g.shader().uniform("layerSeparation", mLayerSeparation);
  g.shader().uniform("is_omni", 1.0f);
  g.shader().uniform("eye_sep", scale * g.lens().eyeSep() * g.eye() / 2.0f);
  // g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  g.shader().uniform("foc_len", g.lens().focalLength());
  g.shader().uniform("clipped_mult", 0.45);
  g.update();

  renderInstances(g, scale, mAtomData, mAligned4fData);
}

void AtomRenderer::renderInstances(Graphics &g, float scale,
                                   std::map<std::string, AtomData> &mAtomData,
                                   std::vector<float> &mAligned4fData) {

  int cumulativeCount = 0;
  for (auto data : mAtomData) {
    if (mShowRadius == 1.0f) {
      g.shader().uniform("markerScale", data.second.radius * mAtomMarkerSize *
                                            mMarkerScale / scale);
      //                std::cout << data.radius << std::endl;
    } else {
      g.shader().uniform("markerScale", mAtomMarkerSize * mMarkerScale / scale);
    }
    int count = data.second.counts;
    assert((int)mAligned4fData.size() >= (cumulativeCount + count) * 4);
    instancingMesh.attrib_data(count * 4 * sizeof(float),
                               mAligned4fData.data() + (cumulativeCount * 4),
                               count);
    cumulativeCount += count;

    g.polygonFill();
    g.shader().uniform("is_line", 0.0f);
    instancingMesh.draw();

    g.shader().uniform("is_line", 1.0f);
    g.polygonLine();
    instancingMesh.draw();
    g.polygonFill();
  }
}

void SlicingAtomRenderer::init() {
  AtomRenderer::init();

  mSliceRotationPitch.registerChangeCallback([this](float value) {
    mSlicingPlaneNormal.setNoCalls(Vec3f(sin(mSliceRotationRoll),
                                         cos(mSliceRotationRoll) * sin(value),
                                         cos(value))
                                       .normalize());
  });

  mSliceRotationRoll.registerChangeCallback([this](float value) {
    mSlicingPlaneNormal.setNoCalls(Vec3f(sin(value),
                                         cos(value) * sin(mSliceRotationPitch),
                                         cos(mSliceRotationPitch))
                                       .normalize());
  });

  mSlicingPlaneNormal.registerChangeCallback([this](Vec3f value) {
    value = value.normalized();
    float pitch = std::atan(value.y / value.z);
    float roll = std::atan(value.x / value.z);
    mSliceRotationPitch.setNoCalls(pitch);
    mSliceRotationRoll.setNoCalls(roll);
  });
}

void SlicingAtomRenderer::setDataBoundaries(BoundingBoxData &b) {
  AtomRenderer::setDataBoundaries(b);
  mSlicingPlanePoint.setHint("maxx", b.max.x);
  mSlicingPlanePoint.setHint("minx", b.min.x - (b.max.x));
  mSlicingPlanePoint.setHint("maxy", b.max.y);
  mSlicingPlanePoint.setHint("miny", b.min.y - (b.max.y));
  mSlicingPlanePoint.setHint("maxz", b.max.z);
  mSlicingPlanePoint.setHint("minz", b.min.z - (b.max.z));
  mSlicingPlaneThickness.min(0.0);
  mSlicingPlaneThickness.max(b.max.z - b.min.z);
}

void SlicingAtomRenderer::draw(Graphics &g, float scale,
                               std::map<std::string, AtomData> &mAtomData,
                               std::vector<float> &mAligned4fData) {

  //  int cumulativeCount = 0;
  // now draw data with custom shaderg.shader(instancing_mesh0.shader);
  g.shader(instancingMesh.shader);
  g.shader().uniform("is_omni", 1.0f);
  g.shader().uniform("eye_sep", scale * g.lens().eyeSep() * g.eye() / 2.0f);
  // g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  g.shader().uniform("foc_len", g.lens().focalLength());

  g.shader().uniform("dataScale", 1.0f / ((mSlicingPlanePoint.getHint("maxy") -
                                           mSlicingPlanePoint.getHint("miny")) *
                                          scale));
  g.shader().uniform("layerSeparation", mLayerSeparation);
  g.shader().uniform("plane_point", mSlicingPlanePoint.get());
  g.shader().uniform("plane_normal", mSlicingPlaneNormal.get().normalized());
  g.shader().uniform("second_plane_distance", mSlicingPlaneThickness);

  g.shader().uniform("clipped_mult", 0.45);
  g.update();
  renderInstances(g, scale, mAtomData, mAligned4fData);
}

void SlicingAtomRenderer::nextLayer() {
  mSlicingPlanePoint =
      mSlicingPlanePoint.get() +
      mSlicingPlaneNormal.get().normalized() * mSlicingPlaneThickness;
}

void SlicingAtomRenderer::previousLayer() {
  mSlicingPlanePoint =
      mSlicingPlanePoint.get() -
      mSlicingPlaneNormal.get().normalized() * mSlicingPlaneThickness;
}

void SlicingAtomRenderer::resetSlicing() {
  // Minimum value for hint allows for slice to be completely outside the
  // dataset so:
  auto minz =
      mSlicingPlanePoint.getHint("minz") + mSlicingPlanePoint.getHint("maxz");
  mSlicingPlanePoint.set({0, 0, minz});

  mSlicingPlaneThickness = mSlicingPlaneThickness.max();
  mSliceRotationRoll.set(0);
  mSliceRotationPitch.set(0);
  //      std::cout << mSlicingPlaneThickness.get() <<std::endl;
}
