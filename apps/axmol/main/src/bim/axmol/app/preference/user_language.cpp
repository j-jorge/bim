// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/user_language.hpp>

#include <iscool/iso_639_language_code.hpp>
#include <iscool/language_name.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/system/language_name.hpp>

iscool::language_name
bim::axmol::app::user_language(const iscool::preferences::local_preferences& p)
{
  const std::string language_string =
      p.get_value("user_language", std::string());
  iscool::language_name language_name;

  if (language_string == "")
    language_name = iscool::system::get_language_name();
  else
    language_name = iscool::from_string_with_fallback(language_string);

  if (language_name == iscool::language_name::pt_BR)
    return language_name;

  switch (iscool::to_language_code(language_name))
    {
    case iscool::iso_639_language_code::br:
      return iscool::language_name::br_FR;
    case iscool::iso_639_language_code::de:
      return iscool::language_name::de_DE;
    case iscool::iso_639_language_code::en:
      return iscool::language_name::en_GB;
    case iscool::iso_639_language_code::fr:
      return iscool::language_name::fr_FR;
    case iscool::iso_639_language_code::pt:
      return iscool::language_name::pt_PT;
    default:
      return iscool::language_name::en_GB;
    }
}

void bim::axmol::app::user_language(iscool::preferences::local_preferences& p,
                                    iscool::language_name language)
{
  p.set_value("user_language", std::string(iscool::to_string(language)));
}
