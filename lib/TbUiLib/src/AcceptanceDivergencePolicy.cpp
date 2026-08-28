/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceDivergencePolicy.h"

#include <QJsonArray>
#include <QJsonValue>

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace tb::ui
{
namespace
{

AcceptanceDivergenceError error(std::string message)
{
  return {std::move(message)};
}

bool finite(const vm::vec3d& value)
{
  return std::isfinite(value.x()) && std::isfinite(value.y()) && std::isfinite(value.z());
}

QJsonArray vecToJson(const vm::vec3d& value)
{
  return {value.x(), value.y(), value.z()};
}

std::optional<vm::vec3d> vecFromJson(const QJsonValue& value)
{
  if (!value.isArray())
    return std::nullopt;
  const auto array = value.toArray();
  if (
    array.size() != 3 || !array[0].isDouble() || !array[1].isDouble()
    || !array[2].isDouble())
    return std::nullopt;
  const auto result =
    vm::vec3d{array[0].toDouble(), array[1].toDouble(), array[2].toDouble()};
  return finite(result) ? std::optional{result} : std::nullopt;
}

QJsonObject boundsToJson(const vm::bbox3d& bounds)
{
  return {{"min", vecToJson(bounds.min)}, {"max", vecToJson(bounds.max)}};
}

std::optional<vm::bbox3d> boundsFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return std::nullopt;
  const auto object = value.toObject();
  const auto min = vecFromJson(object.value("min"));
  const auto max = vecFromJson(object.value("max"));
  if (!min || !max)
    return std::nullopt;
  const auto result = vm::bbox3d{*min, *max};
  return result.is_valid() && !result.is_empty() ? std::optional{result} : std::nullopt;
}

const char* toString(const AcceptanceDivergenceDomain value)
{
  switch (value)
  {
  case AcceptanceDivergenceDomain::SolidSpace:
    return "solidSpace";
  case AcceptanceDivergenceDomain::Visual:
    return "visual";
  case AcceptanceDivergenceDomain::Traversal:
    return "traversal";
  }
  return "";
}

const char* toString(const AcceptanceDivergenceDirection value)
{
  switch (value)
  {
  case AcceptanceDivergenceDirection::ReferenceOnly:
    return "referenceOnly";
  case AcceptanceDivergenceDirection::CandidateOnly:
    return "candidateOnly";
  case AcceptanceDivergenceDirection::Different:
    return "different";
  }
  return "";
}

const char* toString(const AcceptanceDivergenceDisposition value)
{
  switch (value)
  {
  case AcceptanceDivergenceDisposition::Review:
    return "review";
  case AcceptanceDivergenceDisposition::IntendedChange:
    return "intendedChange";
  case AcceptanceDivergenceDisposition::AcceptedRepair:
    return "acceptedRepair";
  case AcceptanceDivergenceDisposition::Waived:
    return "waived";
  }
  return "";
}

const char* toString(const AcceptanceDivergenceSeverity value)
{
  switch (value)
  {
  case AcceptanceDivergenceSeverity::Info:
    return "info";
  case AcceptanceDivergenceSeverity::Warning:
    return "warning";
  case AcceptanceDivergenceSeverity::Error:
    return "error";
  }
  return "";
}

const char* toString(const AcceptanceDivergenceProvenance value)
{
  switch (value)
  {
  case AcceptanceDivergenceProvenance::ManualReview:
    return "manualReview";
  case AcceptanceDivergenceProvenance::ExportDefect:
    return "exportDefect";
  case AcceptanceDivergenceProvenance::DesignDecision:
    return "designDecision";
  case AcceptanceDivergenceProvenance::KnownLimitation:
    return "knownLimitation";
  }
  return "";
}

template <typename T>
std::optional<T> enumFromString(const QString& value);

template <>
std::optional<AcceptanceDivergenceDomain> enumFromString(const QString& value)
{
  if (value == "solidSpace")
    return AcceptanceDivergenceDomain::SolidSpace;
  if (value == "visual")
    return AcceptanceDivergenceDomain::Visual;
  if (value == "traversal")
    return AcceptanceDivergenceDomain::Traversal;
  return std::nullopt;
}

template <>
std::optional<AcceptanceDivergenceDirection> enumFromString(const QString& value)
{
  if (value == "referenceOnly")
    return AcceptanceDivergenceDirection::ReferenceOnly;
  if (value == "candidateOnly")
    return AcceptanceDivergenceDirection::CandidateOnly;
  if (value == "different")
    return AcceptanceDivergenceDirection::Different;
  return std::nullopt;
}

template <>
std::optional<AcceptanceDivergenceDisposition> enumFromString(const QString& value)
{
  if (value == "review")
    return AcceptanceDivergenceDisposition::Review;
  if (value == "intendedChange")
    return AcceptanceDivergenceDisposition::IntendedChange;
  if (value == "acceptedRepair")
    return AcceptanceDivergenceDisposition::AcceptedRepair;
  if (value == "waived")
    return AcceptanceDivergenceDisposition::Waived;
  return std::nullopt;
}

template <>
std::optional<AcceptanceDivergenceSeverity> enumFromString(const QString& value)
{
  if (value == "info")
    return AcceptanceDivergenceSeverity::Info;
  if (value == "warning")
    return AcceptanceDivergenceSeverity::Warning;
  if (value == "error")
    return AcceptanceDivergenceSeverity::Error;
  return std::nullopt;
}

template <>
std::optional<AcceptanceDivergenceProvenance> enumFromString(const QString& value)
{
  if (value == "manualReview")
    return AcceptanceDivergenceProvenance::ManualReview;
  if (value == "exportDefect")
    return AcceptanceDivergenceProvenance::ExportDefect;
  if (value == "designDecision")
    return AcceptanceDivergenceProvenance::DesignDecision;
  if (value == "knownLimitation")
    return AcceptanceDivergenceProvenance::KnownLimitation;
  return std::nullopt;
}

bool contains(const vm::bbox3d& outer, const vm::bbox3d& inner)
{
  return outer.min.x() <= inner.min.x() && outer.min.y() <= inner.min.y()
         && outer.min.z() <= inner.min.z() && outer.max.x() >= inner.max.x()
         && outer.max.y() >= inner.max.y() && outer.max.z() >= inner.max.z();
}

bool applies(
  const AcceptanceDivergenceRule& rule, const AcceptanceDivergenceFinding& finding)
{
  return rule.domain == finding.domain
         && (!rule.direction || *rule.direction == finding.direction)
         && (!rule.scope || (finding.bounds && contains(*rule.scope, *finding.bounds)));
}

int specificity(const AcceptanceDivergenceRule& rule)
{
  return (rule.direction ? 1 : 0) + (rule.scope ? 2 : 0);
}

AcceptanceDivergenceSeverity severityFor(
  const AcceptanceDivergenceSeverityPolicy& policy,
  const AcceptanceDivergenceDisposition disposition)
{
  switch (disposition)
  {
  case AcceptanceDivergenceDisposition::Review:
    return policy.review;
  case AcceptanceDivergenceDisposition::IntendedChange:
    return policy.intendedChange;
  case AcceptanceDivergenceDisposition::AcceptedRepair:
    return policy.acceptedRepair;
  case AcceptanceDivergenceDisposition::Waived:
    return policy.waived;
  }
  return policy.review;
}

} // namespace

AcceptanceDivergencePolicyValidationResult validateAcceptanceDivergencePolicy(
  const AcceptanceDivergencePolicy& policy)
{
  auto ids = std::set<std::string>{};
  for (const auto& rule : policy.rules)
  {
    if (rule.id.empty() || rule.name.empty() || rule.rationale.empty()
        || (rule.disposition != AcceptanceDivergenceDisposition::Review
            && rule.sourceReference.empty()))
      return error(
        "Divergence rules require id, name, rationale, and an evidence reference for "
        "dispositions other than review");
    if (!ids.insert(rule.id).second)
      return error("Divergence rule ids must be unique");
    if (rule.scope && (!rule.scope->is_valid() || rule.scope->is_empty()
                       || !finite(rule.scope->min) || !finite(rule.scope->max)))
      return error("Divergence rule scope must be finite and nonempty");
  }
  return {};
}

std::vector<AcceptanceDivergenceClassification> classifyAcceptanceDivergences(
  const AcceptanceDivergencePolicy& policy,
  std::vector<AcceptanceDivergenceFinding> findings)
{
  std::ranges::sort(findings, {}, &AcceptanceDivergenceFinding::id);
  auto result = std::vector<AcceptanceDivergenceClassification>{};
  result.reserve(findings.size());
  for (auto& finding : findings)
  {
    const AcceptanceDivergenceRule* selected = nullptr;
    for (const auto& rule : policy.rules)
    {
      if (!applies(rule, finding))
        continue;
      if (
        selected == nullptr || specificity(rule) > specificity(*selected)
        || (specificity(rule) == specificity(*selected) && rule.id < selected->id))
        selected = &rule;
    }
    const auto disposition =
      selected ? selected->disposition : AcceptanceDivergenceDisposition::Review;
    result.push_back(
      {std::move(finding),
       disposition,
       severityFor(policy.severity, disposition),
       selected ? std::optional{selected->id} : std::nullopt});
  }
  return result;
}

QJsonObject acceptanceDivergenceFindingToJson(const AcceptanceDivergenceFinding& finding)
{
  auto result = QJsonObject{
    {"id", QString::fromStdString(finding.id)},
    {"domain", toString(finding.domain)},
    {"direction", toString(finding.direction)},
    {"count", static_cast<qint64>(finding.count)}};
  if (finding.bounds)
    result.insert("bounds", boundsToJson(*finding.bounds));
  return result;
}

QJsonObject acceptanceDivergenceClassificationToJson(
  const AcceptanceDivergenceClassification& classification)
{
  auto result = QJsonObject{
    {"finding", acceptanceDivergenceFindingToJson(classification.finding)},
    {"disposition", toString(classification.disposition)},
    {"severity", toString(classification.severity)}};
  if (classification.ruleId)
    result.insert("ruleId", QString::fromStdString(*classification.ruleId));
  return result;
}

QJsonObject acceptanceDivergencePolicyToJson(const AcceptanceDivergencePolicy& policy)
{
  auto rules = policy.rules;
  std::ranges::sort(rules, {}, &AcceptanceDivergenceRule::id);
  auto ruleJson = QJsonArray{};
  for (const auto& rule : rules)
  {
    auto object = QJsonObject{
      {"id", QString::fromStdString(rule.id)},
      {"name", QString::fromStdString(rule.name)},
      {"domain", toString(rule.domain)},
      {"disposition", toString(rule.disposition)},
      {"provenance", toString(rule.provenance)},
      {"sourceReference", QString::fromStdString(rule.sourceReference)},
      {"rationale", QString::fromStdString(rule.rationale)}};
    if (rule.direction)
      object.insert("direction", toString(*rule.direction));
    if (rule.scope)
      object.insert("scope", boundsToJson(*rule.scope));
    ruleJson.append(object);
  }
  return {
    {"severity",
     QJsonObject{
       {"review", toString(policy.severity.review)},
       {"intendedChange", toString(policy.severity.intendedChange)},
       {"acceptedRepair", toString(policy.severity.acceptedRepair)},
       {"waived", toString(policy.severity.waived)}}},
    {"rules", ruleJson}};
}

AcceptanceDivergencePolicyResult acceptanceDivergencePolicyFromJson(
  const QJsonObject& json)
{
  const auto severityObject = json.value("severity");
  const auto rulesValue = json.value("rules");
  if (!severityObject.isObject() || !rulesValue.isArray())
    return error("Divergence policy requires severity and rules");
  auto policy = AcceptanceDivergencePolicy{};
  const auto severity = severityObject.toObject();
  const auto readSeverity = [&](const char* key, AcceptanceDivergenceSeverity& output) {
    const auto value = severity.value(key);
    const auto parsed = value.isString()
                          ? enumFromString<AcceptanceDivergenceSeverity>(value.toString())
                          : std::nullopt;
    if (!parsed)
      return false;
    output = *parsed;
    return true;
  };
  if (
    !readSeverity("review", policy.severity.review)
    || !readSeverity("intendedChange", policy.severity.intendedChange)
    || !readSeverity("acceptedRepair", policy.severity.acceptedRepair)
    || !readSeverity("waived", policy.severity.waived))
    return error("Divergence policy has an invalid severity");

  for (const auto& value : rulesValue.toArray())
  {
    if (!value.isObject())
      return error("Divergence rule must be an object");
    const auto object = value.toObject();
    const auto domain =
      enumFromString<AcceptanceDivergenceDomain>(object.value("domain").toString());
    const auto disposition = enumFromString<AcceptanceDivergenceDisposition>(
      object.value("disposition").toString());
    const auto provenance = enumFromString<AcceptanceDivergenceProvenance>(
      object.value("provenance").toString());
    if (
      !domain || !disposition || !provenance || !object.value("id").isString()
      || !object.value("name").isString() || !object.value("rationale").isString()
      || !object.value("sourceReference").isString())
      return error("Divergence rule has invalid required fields");
    auto rule = AcceptanceDivergenceRule{
      object.value("id").toString().toStdString(),
      object.value("name").toString().toStdString(),
      *domain,
      std::nullopt,
      std::nullopt,
      *disposition,
      *provenance,
      object.value("sourceReference").toString().toStdString(),
      object.value("rationale").toString().toStdString()};
    if (object.contains("direction"))
    {
      const auto direction = enumFromString<AcceptanceDivergenceDirection>(
        object.value("direction").toString());
      if (!direction)
        return error("Divergence rule has invalid direction");
      rule.direction = *direction;
    }
    if (object.contains("scope"))
    {
      const auto scope = boundsFromJson(object.value("scope"));
      if (!scope)
        return error("Divergence rule has invalid scope");
      rule.scope = *scope;
    }
    policy.rules.push_back(std::move(rule));
  }
  const auto validation = validateAcceptanceDivergencePolicy(policy);
  if (validation.is_error())
    return std::get<AcceptanceDivergenceError>(validation.error());
  return policy;
}

} // namespace tb::ui
