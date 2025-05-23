#include "apollo/apollo_types.h"

namespace apollo
{
namespace client
{

Changes ConfiguresDiff(const Configures& old_config, const Configures& new_config)
{
    Changes changes;

    // Check for added or updated items
    for (const auto& pair : new_config)
    {
        auto it = old_config.find(pair.first);
        if (it == old_config.end())
        {
            // New item added
            changes.push_back({ChangeType::ChangeTypeAdd, pair.first, pair.second});
        }
        else if (it->second != pair.second)
        {
            // Existing item updated
            changes.push_back({ChangeType::ChangeTypeUpdate, pair.first, pair.second});
        }
    }

    // Check for deleted items
    for (const auto& pair : old_config)
    {
        if (new_config.find(pair.first) == new_config.end())
        {
            // Item deleted
            changes.push_back({ChangeType::ChangeTypeDelete, pair.first, pair.second});
        }
    }

    return changes;
}
}  // namespace client
}  // namespace apollo