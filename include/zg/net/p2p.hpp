#pragma once
#include "udp_client.hpp"
#include "udp_server.hpp"
#include "udpmc_receiver.hpp"
#include "udpmc_sender.hpp"
namespace zg::net
{
#define PARTY_INFO_NAME_LENGTH 33
#define LOBbY_INFO_NAME_LENGTH 88
	struct p2p
	{

	public:
		struct PartyInfo
		{
			unsigned char name[PARTY_INFO_NAME_LENGTH];
			unsigned int playerCount;
		};

		struct LobbyInfo
		{
			unsigned char name[LOBbY_INFO_NAME_LENGTH];
			unsigned long long playerCount;
			bool started;
			bool finished;
			bool regame;
		};

	public:
		struct Party
		{
			std::shared_ptr<PartyInfo> info;
			// connect members of party
		};
		struct Lobby
		{
			std::shared_ptr<LobbyInfo> info;
			// connect members of lobby
		};
		using PartyMap = std::map<UniqueIdentifier, std::shared_ptr<Party>>;
		using LobbyMap = std::map<UniqueIdentifier, std::shared_ptr<Lobby>>;

	protected:
		struct host_info_factory;
		struct HostInfo
		{
			friend p2p;
			friend host_info_factory;
			// gpos is used for distance calculations and lobby/party coordinations
		protected:
			long* hostPlanet = (long*)1;
			long double latitude;
			long double longitude;
			std::pair<UniqueIdentifier, PartyMap> partyMapPair;
			std::pair<UniqueIdentifier, LobbyMap> lobbyMapPair;
		};
		struct host_info_factory
		{
			static HostInfo create();
		};

	private:
		HostInfo hostInfo = host_info_factory::create();
		std::string announceIP;
		std::shared_ptr<udpmc_sender> announceSender;
		std::shared_ptr<udpmc_receiver> announceReceiver;
		std::unordered_map<UniqueIdentifier, PartyInfo> partyInfos;
		std::unordered_map<UniqueIdentifier, LobbyInfo> lobbyInfos;

	public:
		p2p(const std::string& announceIP);
		bool doingAnnounce();
		void startAnnounce();
		void closeAnnounce();
	};
} // namespace zg::net
