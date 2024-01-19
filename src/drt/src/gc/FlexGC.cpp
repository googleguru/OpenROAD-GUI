/* Authors: Lutong Wang and Bangqi Xu */
/*
 * Copyright (c) 2019, The Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>

#include "gc/FlexGC_impl.h"

using namespace fr;

FlexGCWorker::FlexGCWorker(frTechObject* techIn,
                           Logger* logger,
                           FlexDRWorker* drWorkerIn)
    : impl_(std::make_unique<Impl>(techIn, logger, drWorkerIn, this))
{
}

FlexGCWorker::~FlexGCWorker() = default;

FlexGCWorker::Impl::Impl() : Impl(nullptr, nullptr, nullptr, nullptr)
{
}

FlexGCWorker::Impl::Impl(frTechObject* techIn,
                         Logger* logger,
                         FlexDRWorker* drWorkerIn,
                         FlexGCWorker* gcWorkerIn)
    : tech_(techIn),
      logger_(logger),
      drWorker_(drWorkerIn),
      extBox_(),
      drcBox_(),
      owner2nets_(),
      nets_(),
      markers_(),
      mapMarkers_(),
      pwires_(),
      rq_(gcWorkerIn),
      printMarker_(false),
      modifiedDRNets_(),
      targetNet_(nullptr),
      minLayerNum_(std::numeric_limits<frLayerNum>::min()),
      maxLayerNum_(std::numeric_limits<frLayerNum>::max()),
      ignoreDB_(false),
      ignoreMinArea_(false),
      ignoreLongSideEOL_(false),
      ignoreCornerSpacing_(false),
      surgicalFixEnabled_(false)
{
}

void FlexGCWorker::Impl::addMarker(std::unique_ptr<frMarker> in)
{
  Rect bbox = in->getBBox();
  auto layerNum = in->getLayerNum();
  auto con = in->getConstraint();
  std::vector<frBlockObject*> srcs(2, nullptr);
  int i = 0;
  for (auto& src : in->getSrcs()) {
    srcs.at(i) = src;
    i++;
  }
  if (mapMarkers_.find({bbox, layerNum, con, srcs[0], srcs[1]})
      != mapMarkers_.end()) {
    return;
  }
  if (mapMarkers_.find({bbox, layerNum, con, srcs[1], srcs[0]})
      != mapMarkers_.end()) {
    return;
  }
  mapMarkers_[{bbox, layerNum, con, srcs[0], srcs[1]}] = in.get();
  markers_.push_back(std::move(in));
}

void FlexGCWorker::addPAObj(frConnFig* obj, frBlockObject* owner)
{
  impl_->addPAObj(obj, owner);
}

void FlexGCWorker::init(const frDesign* design)
{
  impl_->init(design);
}

int FlexGCWorker::main()
{
  return impl_->main();
}

void FlexGCWorker::checkMinStep(gcPin* pin)
{
  impl_->checkMetalShape_minStep(pin);
}

void FlexGCWorker::updateGCWorker()
{
  impl_->updateGCWorker();
}

void FlexGCWorker::end()
{
  impl_->end();
}

void FlexGCWorker::initPA0(const frDesign* design)
{
  impl_->initPA0(design);
}

void FlexGCWorker::initPA1()
{
  impl_->initPA1();
}

void FlexGCWorker::setExtBox(const Rect& in)
{
  impl_->extBox_ = in;
}

void FlexGCWorker::setDrcBox(const Rect& in)
{
  impl_->drcBox_ = in;
}

const std::vector<std::unique_ptr<frMarker>>& FlexGCWorker::getMarkers() const
{
  return impl_->markers_;
}

const std::vector<std::unique_ptr<drPatchWire>>& FlexGCWorker::getPWires() const
{
  return impl_->pwires_;
}

void FlexGCWorker::clearPWires()
{
  impl_->pwires_.clear();
}

bool FlexGCWorker::setTargetNet(frBlockObject* in)
{
  auto& owner2nets = impl_->owner2nets_;
  if (owner2nets.find(in) != owner2nets.end()) {
    impl_->targetNet_ = owner2nets[in];
    return true;
  } else {
    return false;
  }
}
gcNet* FlexGCWorker::getTargetNet()
{
  return impl_->targetNet_;
}
void FlexGCWorker::setEnableSurgicalFix(bool in)
{
  impl_->surgicalFixEnabled_ = in;
}

void FlexGCWorker::resetTargetNet()
{
  impl_->targetNet_ = nullptr;
}

void FlexGCWorker::addTargetObj(frBlockObject* in)
{
  impl_->targetObjs_.insert(in);
}

void FlexGCWorker::setTargetObjs(const std::set<frBlockObject*>& targetObjs)
{
  impl_->targetObjs_ = targetObjs;
}

void FlexGCWorker::setIgnoreDB()
{
  impl_->ignoreDB_ = true;
}

void FlexGCWorker::setIgnoreMinArea()
{
  impl_->ignoreMinArea_ = true;
}

void FlexGCWorker::setIgnoreCornerSpacing()
{
  impl_->ignoreCornerSpacing_ = true;
}

void FlexGCWorker::setIgnoreLongSideEOL()
{
  impl_->ignoreLongSideEOL_ = true;
}

std::vector<std::unique_ptr<gcNet>>& FlexGCWorker::getNets()
{
  return impl_->getNets();
}

gcNet* FlexGCWorker::getNet(frNet* net)
{
  return impl_->getNet(net);
}