#include "SceneConfigViewModel.hpp"

namespace viewmodels {

SceneConfigViewModel::SceneConfigViewModel(QObject* parent)
    : QObject(parent)
    , config_()
{
}

SceneConfigViewModel::SceneConfigViewModel(const config::SceneConfig& config, QObject* parent)
    : QObject(parent)
    , config_(config)
{
}

} // namespace viewmodels
