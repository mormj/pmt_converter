#pragma once

#include <legacy/pmt.h>
#include "pmt_converter.h"
#include <pmtv/pmt.hpp>

namespace gr_compat {

pmtv::pmt to_new_pmt(const legacy::pmt_t& old) {
    if (old.is_bool()) {
        return pmtv::pmt(old.to_bool());
    } else if (old.is_int()) {
        return pmtv::pmt(old.to_int());
    } else if (old.is_symbol()) {
        return pmtv::pmt(symbol(old.to_symbol()));
    } else if (old.is_pair()) {
        return pmtv::pmt(pair(
            to_new_pmt(old.car()),
            to_new_pmt(old.cdr())));
    } else if (old.is_vector()) {
        std::vector<pmtv::pmt> vec;
        for (const auto& item : old.to_vector()) {
            vec.push_back(to_new_pmt(item));
        }
        return pmtv::pmt(vector(vec));
    } else if (old.is_dict()) {
        map m;
        for (const auto& [k, v] : old.to_dict()) {
            m.set(to_new_pmt(k), to_new_pmt(v));
        }
        return pmtv::pmt(m);
    } else {
        throw std::runtime_error("Unsupported legacy PMT type");
    }
}

legacy::pmt_t to_legacy_pmt(const pmtv::pmt& obj) {
    if (obj.is<bool>()) {
        return legacy::make_bool(obj.get<bool>());
    } else if (obj.is<int64_t>()) {
        return legacy::make_int(obj.get<int64_t>());
    } else if (obj.is<symbol>()) {
        return legacy::make_symbol(obj.get<symbol>().str());
    } else if (obj.is<pair>()) {
        auto p = obj.get<pair>();
        return legacy::make_pair(
            to_legacy_pmt(p.first),
            to_legacy_pmt(p.second));
    } else if (obj.is<vector>()) {
        std::vector<legacy::pmt_t> vec;
        for (const auto& item : obj.get<vector>().items) {
            vec.push_back(to_legacy_pmt(item));
        }
        return legacy::make_vector(vec);
    } else if (obj.is<map>()) {
        legacy::dict_type dict;
        for (const auto& [k, v] : obj.get<map>().items) {
            dict[to_legacy_pmt(k)] = to_legacy_pmt(v);
        }
        return legacy::make_dict(dict);
    } else {
        throw std::runtime_error("Unsupported PMT4 type");
    }
}

}
