#pragma once
#define ﻿CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

struct HostPath
{
	std::string Host;
	std::string Path;

	HostPath() {}

	HostPath(const std::string& host, const std::string& path)
	{
		Host = host;
		Path = path;
	}

	HostPath(const std::string& link)
	{
		int slashCount = 0;

		for (int i = 0; i < link.length(); i++)
		{
			if (slashCount == 3)
			{
				Host = link.substr(0, i - 1);
				Path = link.substr(i - 1);
				break;
			}

			if (link[i] == '/')
			{
				slashCount++;
			}
		}
	}

	bool operator==(HostPath right)
	{
		return (this->Host == right.Host) && (this->Path == right.Path);
	}
};

void DownloadFile(const std::string& url, const std::string& outFilePath)
{
	HostPath fileUrl(url);

	/* create client for the host */
	httplib::Client downloadClient(fileUrl.Host);

	/* set properties */
	downloadClient.set_follow_location(false);
	downloadClient.set_keep_alive(false);
	downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	std::ofstream downloadFileStream(outFilePath);

	printf("\033[ 7");

	httplib::Result res = downloadClient.Get(fileUrl.Path,
		[&](const char* data, size_t data_length)
		{
			/* write to file while downloading, this makes sure that it doesn't download to memory and then write */
			downloadFileStream.write(data, data_length);
			return true;
		},
		[&](uint64_t len, uint64_t total)
		{
			printf("%f Done", float(len) / float(total));
			printf("\033[ 8");
			return true; // return 'false' if you want to cancel the request.
		});

	downloadFileStream.close();

	return;
}