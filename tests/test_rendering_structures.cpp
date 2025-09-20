#include <array>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "pandolabo.hpp"

using namespace pandora::core;

TEST_CASE("AttachmentDescription fluent setters", "[render][attachment]") {
  AttachmentDescription ad{};
  ad.setFormat(DataFormat::R8G8B8A8Unorm)
      .setSamples(ImageSampleCount::v1)
      .setLoadOp(AttachmentLoadOp::Clear)
      .setStoreOp(AttachmentStoreOp::Store)
      .setStencilLoadOp(AttachmentLoadOp::DontCare)
      .setStencilStoreOp(AttachmentStoreOp::DontCare)
      .setLayouts(ImageLayout::Undefined, ImageLayout::PresentSrc)
      .setStencilLayouts(ImageLayout::Undefined,
                         ImageLayout::DepthStencilAttachmentOptimal);

  REQUIRE(ad.format == DataFormat::R8G8B8A8Unorm);
  REQUIRE(ad.samples == ImageSampleCount::v1);
  REQUIRE(ad.load_op == AttachmentLoadOp::Clear);
  REQUIRE(ad.store_op == AttachmentStoreOp::Store);
  REQUIRE(ad.stencil_load_op == AttachmentLoadOp::DontCare);
  REQUIRE(ad.stencil_store_op == AttachmentStoreOp::DontCare);
  REQUIRE(ad.initial_layout == ImageLayout::Undefined);
  REQUIRE(ad.final_layout == ImageLayout::PresentSrc);
  REQUIRE(ad.stencil_initial_layout == ImageLayout::Undefined);
  REQUIRE(ad.stencil_final_layout
          == ImageLayout::DepthStencilAttachmentOptimal);
}

TEST_CASE("AttachmentReference fluent setters", "[render][attachment]") {
  AttachmentReference ar{};
  ar.setIndex(3).setLayout(ImageLayout::ColorAttachmentOptimal);

  REQUIRE(ar.index == 3);
  REQUIRE(ar.layout == ImageLayout::ColorAttachmentOptimal);
}

TEST_CASE("SubpassEdge fluent setters and adders", "[render][subpass]") {
  SubpassEdge e{};
  e.setSrcIndex(VK_SUBPASS_EXTERNAL)
      .setDstIndex(1)
      .setDependencyFlag(DependencyFlag::ByRegion)
      .setSrcStages({PipelineStage::ColorAttachmentOutput})
      .setDstStages({PipelineStage::FragmentShader})
      .setSrcAccess({AccessFlag::ColorAttachmentWrite})
      .setDstAccess({AccessFlag::ShaderRead});

  e.addSrcStage(PipelineStage::Transfer)
      .addDstStage(PipelineStage::VertexShader)
      .addSrcAccess(AccessFlag::TransferWrite)
      .addDstAccess(AccessFlag::ShaderWrite);

  REQUIRE(e.src_index == VK_SUBPASS_EXTERNAL);
  REQUIRE(e.dst_index == 1u);
  REQUIRE(e.dependency_flag == DependencyFlag::ByRegion);
  REQUIRE_FALSE(e.src_stages.empty());
  REQUIRE_FALSE(e.dst_stages.empty());
  REQUIRE_FALSE(e.src_access.empty());
  REQUIRE_FALSE(e.dst_access.empty());
}

TEST_CASE("StencilOpState fluent setters", "[render][stencil]") {
  StencilOpState s{};
  s.setOps(StencilOp::Replace,
           StencilOp::IncrementAndClamp,
           StencilOp::DecrementAndClamp)
      .setCompareOp(CompareOp::Greater)
      .setMasks(0xAA, 0x55)
      .setReference(42);

  REQUIRE(s.fail_op == StencilOp::Replace);
  REQUIRE(s.pass_op == StencilOp::IncrementAndClamp);
  REQUIRE(s.depth_fail_op == StencilOp::DecrementAndClamp);
  REQUIRE(s.compare_op == CompareOp::Greater);
  REQUIRE(s.compare_mask == 0xAA);
  REQUIRE(s.write_mask == 0x55);
  REQUIRE(s.reference == 42u);
}

TEST_CASE("ColorBlendAttachment fluent setters", "[render][blend]") {
  ColorBlendAttachment c{};
  c.setEnabled(true)
      .setColorBlend(
          BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendOp::Add)
      .setAlphaBlend(BlendFactor::One, BlendFactor::Zero, BlendOp::Add)
      .setColorComponents({ColorComponent::R,
                           ColorComponent::G,
                           ColorComponent::B,
                           ColorComponent::A});

  REQUIRE(c.is_enabled);
  REQUIRE(c.src_color == BlendFactor::SrcAlpha);
  REQUIRE(c.dst_color == BlendFactor::OneMinusSrcAlpha);
  REQUIRE(c.color_op == BlendOp::Add);
  REQUIRE(c.src_alpha == BlendFactor::One);
  REQUIRE(c.dst_alpha == BlendFactor::Zero);
  REQUIRE(c.alpha_op == BlendOp::Add);
  REQUIRE(c.color_components.size() == 4);
}

TEST_CASE("ComputeWorkGroupSize setters", "[compute]") {
  ComputeWorkGroupSize w{1, 1, 1};
  w.setSize(8, 4, 2);
  REQUIRE(w.x == 8u);
  REQUIRE(w.y == 4u);
  REQUIRE(w.z == 2u);

  w.setX(16).setY(32).setZ(64);
  REQUIRE(w.x == 16u);
  REQUIRE(w.y == 32u);
  REQUIRE(w.z == 64u);
}

TEST_CASE("ClearColor and ClearDepthStencil setters", "[render][clear]") {
  ClearColor cc{{0.f, 0.f, 0.f, 1.f}};
  cc.setRed(0.1f).setGreen(0.2f).setBlue(0.3f).setAlpha(0.4f);
  REQUIRE(cc.color[0] == 0.1f);
  REQUIRE(cc.color[1] == 0.2f);
  REQUIRE(cc.color[2] == 0.3f);
  REQUIRE(cc.color[3] == 0.4f);

  cc.setColor(1.f, 0.5f, 0.25f, 0.75f);
  REQUIRE(cc.color[0] == 1.0f);
  REQUIRE(cc.color[1] == 0.5f);
  REQUIRE(cc.color[2] == 0.25f);
  REQUIRE(cc.color[3] == 0.75f);

  ClearDepthStencil cds{};
  cds.setValues(1.0f, 255u);
  REQUIRE(cds.depth == 1.0f);
  REQUIRE(cds.stencil == 255u);
}
