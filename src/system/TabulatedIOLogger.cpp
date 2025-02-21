#include <zg/system/TabulatedIOLogger.hpp>
#include <zg/system/Budget.hpp>
using namespace zg::system;
TabulatedIOLogger::TabulatedIOLogger(bool savingTable, bool borders):
    m_savingTable(savingTable),
    m_borders(borders),
    m_seTio(true)
{
    m_seTio.echo(true);
    m_seTio.canonical(true);
    m_seTio.setProfile();
    m_seTsz = m_seTio.getDims();
    m_crTps = m_seTio.getCursor();
    m_seTio << "m_seTsz: " << m_seTsz.x << ", " << m_seTsz.y << "\n"
        << "m_crTpsn: " << m_crTps.x << ", " << m_crTps.y << "\n";
}
TabulatedIOLogger::~TabulatedIOLogger()
{
    if(m_savingTable)
    {
        save_taBle();
    }
}
// void TabulatedIOLogger::log(const string& key, const string& value)
// {
// }
void TabulatedIOLogger::save_taBle()
{
}