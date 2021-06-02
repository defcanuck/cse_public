#include "PCH.h"

#include "serial/Member.h"
#include "serial/MetaData.h"

namespace cs
{
	const MetaData* Member::getComboMeta(const std::string& combo_name) 
	{
		ComboxBoxValues::iterator it = this->combo.find(combo_name);
		if (it != this->combo.end() && it->second.meta_data != nullptr)
			return it->second.meta_data;

		const std::string& comboLookupName = this->combo[combo_name].meta_name;
		const MetaData* metadata = MetaManager::getInstance()->get(comboLookupName);
		this->combo[combo_name].meta_data = metadata;
		return this->combo[combo_name].meta_data;
	}

	size_t Member::getComboMetaLabels(StringList& label_list) const
	{
		label_list.clear();
		for (auto it : this->combo)
			label_list.push_back(it.first);
		return label_list.size();
	}

	size_t Member::getNumComboMeta() const
	{
		return this->combo.size();
	}
    
    void Member::addComboMeta(MetaData* meta, const std::string& combo_name, const std::string & meta_name)
    {
        if (!meta || !meta->getInitialized())
        {
#if defined(CS_WINDOWS)
            log::info(combo_name, " added but meta not initialized yet.  Add null and query again on use");
#endif
            meta = nullptr;
        }
        assert(this->combo.find(combo_name) == this->combo.end());
        this->combo[combo_name].meta_data = meta;
        this->combo[combo_name].meta_name = meta_name;
    }
}
