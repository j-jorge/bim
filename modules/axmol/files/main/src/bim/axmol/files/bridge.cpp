#include <bim/axmol/files/bridge.hpp>

#include <iscool/files/file_system_delegates.hpp>
#include <iscool/files/setup.hpp>

#include <axmol/platform/FileUtils.h>

#include <sstream>

class bim::axmol::files::bridge::delegates final
  : public iscool::files::file_system_delegates
{
public:
  std::unique_ptr<std::istream>
  read_file(const std::string& path) const override;

  std::string get_writable_path() const override;
  bool file_exists(const std::string& path) const override;
  std::string get_full_path(const std::string& path) const override;
};

bim::axmol::files::bridge::bridge()
  : m_delegates(new delegates())
{
  iscool::files::initialize(*m_delegates);
}

bim::axmol::files::bridge::~bridge()
{
  iscool::files::finalize();
}

std::unique_ptr<std::istream>
bim::axmol::files::bridge::delegates::read_file(const std::string& path) const
{
  ax::Data file_content(ax::FileUtils::getInstance()->getDataFromFile(path));

  if (file_content.isNull())
    return std::unique_ptr<std::istream>(new std::istringstream());

  const unsigned char* const bytes(file_content.getBytes());
  std::unique_ptr<std::istream> result(new std::stringstream(
      std::string(bytes, bytes + file_content.getSize())));

  return result;
}

std::string bim::axmol::files::bridge::delegates::get_writable_path() const
{
  return ax::FileUtils::getInstance()->getWritablePath();
}

bool bim::axmol::files::bridge::delegates::file_exists(
    const std::string& path) const
{
  return ax::FileUtils::getInstance()->isFileExist(path);
}

std::string bim::axmol::files::bridge::delegates::get_full_path(
    const std::string& path) const
{
  return ax::FileUtils::getInstance()->fullPathForFilename(path);
}
