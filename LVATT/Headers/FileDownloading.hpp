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
	downloadClient.set_follow_location(true);
	downloadClient.set_keep_alive(true);
	downloadClient.set_default_headers({{"User-Agent", "Project LVATT (cpp-httplib)"}});

	std::ofstream downloadFileStream(outFilePath, std::ios::binary);

	httplib::Result res = downloadClient.Get(fileUrl.Path,
		[&](const char* data, size_t data_length)
		{
			/* write to file while downloading, this makes sure that it doesn't download to memory and then write */
			downloadFileStream.write(data, data_length);
			return true;
		},
		[&](uint64_t len, uint64_t total)
		{
			if (len % 397 != 0 && float(len) / float(total) != 1.0f) /* don't print all the time as it is expensive, just do mod of len with random prime number */
			{
				return true;
			}
			printf("\r%f%%", (float(len) / float(total))*100.0f);
			return true; // return 'false' if you want to cancel the request.
		});
	printf("\n");

	if (!res)
	{
		std::string errorMessage;
		switch (res.error())
		{
		case httplib::Error::Success:
			errorMessage = "Success";
			break;
		case httplib::Error::Unknown:
			errorMessage = "Unknown";
			break;
		case httplib::Error::Connection:
			errorMessage = "Connection";
			break;
		case httplib::Error::BindIPAddress:
			errorMessage = "Bind IP Address";
			break;
		case httplib::Error::Read:
			errorMessage = "Read ";
			break;
		case httplib::Error::Write:
			errorMessage = "Write";
			break;
		case httplib::Error::ExceedRedirectCount:
			errorMessage = "Exceed Redirect Count";
			break;
		case httplib::Error::Canceled:
			errorMessage = "Cancelled";
			break;
		case httplib::Error::SSLConnection:
			errorMessage = "SSL Connection";
			break;
		case httplib::Error::SSLLoadingCerts:
			errorMessage = "SSL Loading Certs";
			break;
		case httplib::Error::SSLServerVerification:
			errorMessage = "SSL Server Verification";
			break;
		case httplib::Error::UnsupportedMultipartBoundaryChars:
			errorMessage = "Unsupported Multipart Boundary Chars";
			break;
		case httplib::Error::Compression:
			errorMessage = "Compression";
			break;
		case httplib::Error::ConnectionTimeout:
			errorMessage = "Connection Timeout";
			break;
		}
		printf((errorMessage + "\n").c_str());
		exit(-1);
	}

	downloadFileStream.close();

	return;
}