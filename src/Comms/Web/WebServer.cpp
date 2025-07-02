#ifndef NO_WEB
#include "WebServer.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>
#include <uptime.h>
#include <ElegantOTA.h>

#include "../../Devices/TFT/HardwareTFT.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include "../../Devices/WiFi/HardwareWiFi.h"
#include "../../Devices/Microphone/HardwareMicrophone.h"
#include "../../Devices/USB/USBCore.h"
#include "../../Devices/USB/USBCDC.h"
#include "../../Attacks/Agent/Agent.h"
#include "../../Debug/Logging.h"

#include "../../Attacks/Marauder/Marauder.h"
#include "../../Attacks/Ducky/DuckyPayload.h"

#include "../../Utilities/Settings.h"
#include "../../version.h"

#define LOG_WEB "WEB"

namespace Comms
{
  WebSite Web;
}

static AsyncWebServer controlInterfaceWebServer(8080);
static AsyncWebSocket ws("/websockify");
static AsyncWebSocket audio("/audio");

extern std::unordered_map<const char *, std::pair<const uint8_t *, size_t>> staticHtmlFilesLookup;
static const char *remoteAddress = "127.0.0.1:7002";
static Preferences *preferences = nullptr;

static const std::unordered_map<std::string, std::string> mimeTypes = {
    {".html", "text/html"},
    {".js", "application/javascript"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".ico", "image/x-icon"},
    {".css", "text/css"},
    {".svg", "image/svg+xml"}};

static std::string prettyPrintUptime()
{
  std::string ret;
  if (uptime::getDays() != 0)
  {
    ret += std::to_string(uptime::getDays()) + " days";
  }
  if (uptime::getHours() != 0)
  {
    ret += std::to_string(uptime::getHours()) + " hours";
  }
  if (uptime::getMinutes() != 0)
  {
    ret += std::to_string(uptime::getMinutes()) + " minutes";
  }
  if (ret.empty())
  {
    ret = "A few seconds";
  }

  return ret;
}

static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    uint32_t remoteAddLen = strlen(remoteAddress);
    Devices::USB::CDC.writeBinary(HostCommand::WSCONNECT, (uint8_t *)remoteAddress, remoteAddLen);
  }
  else if (type == WS_EVT_DATA && data != nullptr && len != 0)
  {
    Devices::USB::CDC.writeBinary(HostCommand::WSDATA, data, len);
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Devices::USB::CDC.writeBinary(HostCommand::WSDISCONNECT, nullptr, 0);
  }
  else
  {
    Devices::USB::CDC.writeDebugString("Got an unhandler websocket event: " + std::to_string(type));
  }
}

static void onWsAudioEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_DISCONNECT)
  {
    Devices::Mic.stopCapture();
  }
}


static std::pair<const uint8_t *, size_t> getStaticHtml(const String &url)
{
  for (const auto &[key, value] : staticHtmlFilesLookup)
  {
    if (url == key)
    {
      return value;
    }
  }
  return std::make_pair<const uint8_t *, size_t>(nullptr, 0);
}

static const char *GetMimeType(const char *fname)
{
  std::string filename(fname);
  // Extract the file extension
  size_t dotPos = filename.find_last_of('.');
  if (dotPos == std::string::npos)
  {
    // No extension found
    return nullptr;
  }

  std::string extension = filename.substr(dotPos);

  // Look up the MIME type
  auto it = mimeTypes.find(extension);
  if (it != mimeTypes.end())
  {
    return it->second.c_str();
  }
  else
  {
    // Extension not found in our mapping
    return nullptr;
  }
}

static void webRequestHandler(AsyncWebServerRequest *request)
{
  const auto &url = request->url();
  // Debug::Log.info(LOG_WEB, std::string("webRequestHandler ") + url.c_str());

  if (url == "/wpad.dat")
  {
    request->send(404);
  }
  else if (url == "/connecttest.txt")
  {
    request->send(404);
  }
  else if (url == "/data.json")
  {
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    root["sdCardPercentFull"] = Devices::Storage.usedPercentage();
    root["uptime"] = prettyPrintUptime();
    root["status"] = Attacks::Ducky.getPayloadRunningStatus();
    root["errorCount"] = Attacks::Ducky.getTotalErrors();
    root["USBmode"] = Devices::USB::Core.getCurrentUSBMode();
    root["freeHeap"] = ESP.getFreeHeap();
    root["heapSize"] = ESP.getHeapSize();
    root["agentConnected"] = Attacks::Agent.isAgentConnected();
    root["machineName"] = Attacks::Agent.machineName();
    root["version"] = GIT_COMMIT_HASH;

    float heapUsed = (float)(ESP.getHeapSize() - ESP.getFreeHeap());
    float totalHeap = (float)ESP.getHeapSize();

    root["heapUsagePc"] = (int)((heapUsed / totalHeap) * 100);
    root["numCores"] = ESP.getChipCores();
    root["chipModel"] = ESP.getChipModel();

    auto fileListing = root["fileListing"].to<JsonArray>();
    for (const auto &filename : Devices::Storage.listFiles())
    {
      fileListing.add(filename);
    }

    auto logMessages = root["logMessages"].to<JsonArray>();
    for (const auto &logEntry : Debug::Log.getLogs())
    {
      logMessages.add(logEntry);
    }

    if (preferences != nullptr)
    {
      auto settingsCategories = root["settingCategories"].to<JsonArray>();
      enumerateSettingsAsJson(*preferences, settingsCategories);
    }

    auto capabilities = root["capabilities"].to<JsonArray>();
#ifndef NO_SD
    capabilities.add("SD");
#endif
#ifndef NO_WIFI
    capabilities.add("WIFI");
#endif
#ifndef NO_TFT
    capabilities.add("TFT");
#endif
#ifndef NO_BUTTON
    capabilities.add("BUTTON");
#endif
#ifndef NO_LED
    capabilities.add("LED");
#endif
#ifndef NO_MIC
    capabilities.add("MIC");
#endif
#ifndef NO_ESP_MARAUDER
    capabilities.add("MARAUDER");
#endif

    response->setLength();
    request->send(response);
  }
  else if (url == "/clearlogs")
  {
    Debug::Log.getLogs().clear();
    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/generate_204")
  {
    request->redirect("/"); // redirect to our main page
  }
  else if (url == "/runfile" && request->hasParam("filename"))
  {
    const String filename = request->getParam("filename")->value();
    Debug::Log.info(LOG_WEB, std::string("Run file") + filename.c_str());

    Attacks::Ducky.setPayload(filename.c_str());
    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/rawinput" && request->hasParam("rawCommand"))
  {
    const String cmdline = request->getParam("rawCommand")->value();
    Debug::Log.info(LOG_WEB, std::string("Running cmdline ") + cmdline.c_str());

    Attacks::Ducky.setPayloadCmdLine(cmdline.c_str());
    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/runagentcmd" && request->hasParam("rawCommand"))
  {
    const String cmdline = request->getParam("rawCommand")->value();
    Debug::Log.info(LOG_WEB, std::string("Running cmd with agent ") + cmdline.c_str());

    Attacks::Ducky.setPayloadCmdLine(std::string("AGENT_RUN ") + cmdline.c_str());
    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/showimage" && request->hasParam("filename"))
  {
    const String filename = request->getParam("filename")->value();
    Debug::Log.info(LOG_WEB, "Show image");
    Debug::Log.info(LOG_WEB, filename.c_str());

    Devices::TFT.displayPng(Devices::Storage, filename.c_str());

    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/marauder" && request->hasParam("marauderCmd"))
  {
    const String cmd = request->getParam("marauderCmd")->value();
    Debug::Log.info(LOG_WEB, std::string("Run Marauder command ") + std::string(cmd.c_str()));
    Attacks::Marauder.run(cmd.c_str());

    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url == "/mic" && request->hasParam("enabled"))
  {
    const String cmd = request->getParam("enabled")->value();
    if (cmd == "true")
    {
      Debug::Log.info(LOG_WEB, "Starting Mic capture");
      Devices::Mic.startCapture();
    }
    else
    {
      Debug::Log.info(LOG_WEB, "Stopping Mic capture");
      Devices::Mic.stopCapture();
    }
    request->send(200);
  }
  else if (url == "/download" && request->hasParam("filename"))
  {
    const String filename = request->getParam("filename")->value();
    Debug::Log.info(LOG_WEB, std::string("In download, filename: ")+filename.c_str());
    auto file = Devices::Storage.openFile(filename.c_str(), "r");

    if (file.available())
    {
      Debug::Log.info(LOG_WEB, std::string("Sending file: ") + filename.c_str());
      AsyncWebServerResponse *response = request->beginResponse(file, filename, "application/octet-stream", true);
      request->send(response);
      // file.close(); // TODO in order for download to work the file can't be closed
      // i've had a look at the code and can't see it closed by beginResponse etc
      // we could be leaking file handles
    }
    else
    {
      Debug::Log.info(LOG_WEB, std::string("File not found: ") + filename.c_str());
      request->send(404);
    }
  }
  else if (url == "/delete" && request->hasParam("filename"))
  {
    const String filename = request->getParam("filename")->value();

    Debug::Log.info(LOG_WEB, std::string("Deleting file: ") + std::string(filename.c_str()));

    Devices::Storage.deleteFile(filename.c_str());

    request->redirect("/index.html"); // redirect to our main page
  }
  else if (url.startsWith("/set") && preferences != nullptr && request->hasParam("name"))
  {
    const String settingName = request->getParam("name")->value();

    if (request->hasParam("value"))
    {
      const String settingValue = request->getParam("value")->value();

      Debug::Log.info(LOG_WEB, std::string("Set setting") + settingName.c_str() + " to " + settingValue.c_str());

      if (setSettingValue(*preferences, settingName.c_str(), settingValue.c_str()))
      {
        Debug::Log.info(LOG_WEB, "New setting has been set");
      }
      else
      {
        Debug::Log.info(LOG_WEB, "Setting could not be set");
      }
    }
    else
    {
      Debug::Log.info(LOG_WEB, "Removing setting");
      preferences->remove(settingName.c_str());
    }
    request->redirect("/index.html"); // redirect to our main page
  }
  else
  {
    std::pair<const uint8_t *, size_t> data;
    const char *unknown = "application/octet-stream";

    if (url.isEmpty() || url.length() == 1)
    {
      data = getStaticHtml("/index.html");
    }
    else
    {
      data = getStaticHtml(url);
    }

    if (data.first != nullptr)
    {
      const char *mime = GetMimeType(url.isEmpty() || url.length() == 1 ? "/index.html" : url.c_str());

      AsyncWebServerResponse *response = request->beginResponse(200, mime != nullptr ? mime : unknown, data.first, data.second);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "max-age=6000");
      request->send(response);
    }
    else
    {
      // Serial.println("sending 404");
      Debug::Log.error(LOG_WEB, std::string("Unknown URL: ") + url.c_str());
      request->send(404);
    }
  }
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  const auto &url = request->url();

  if (url.startsWith("/uploadFile"))
  {
    if (!index) {
    
      // We need to ensure that if the autorun.ds payload is currently running then we stop it
      // Otherwise we risk changing the file content while the script is executing
      if (filename == "autorun.ds")
      {
        Attacks::Ducky.setPayload("");
      }

      request->_tempFile = Devices::Storage.openFile(std::string("/")+filename.c_str(), "w");
    }

    if (len && request->_tempFile.available())
    {
      request->_tempFile.write(data, len);
    }

    if (final)
    {
      // close file
      Debug::Log.info(LOG_WEB, std::string("File uploaded ")+filename.c_str());
      request->_tempFile.close();
      Devices::Storage.refreshCache();
    }
    request->send(200);
  }
}

WebSite::WebSite()
{
}

void WebSite::begin(Preferences &prefs)
{
  if (Devices::WiFi.getState() == false)
  {
    return;
  }

  preferences = &prefs;

  controlInterfaceWebServer.onFileUpload(handleUpload);
  controlInterfaceWebServer.onNotFound(webRequestHandler);
  
  ws.onEvent(onWsEvent);
  audio.onEvent(onWsAudioEvent);

  controlInterfaceWebServer.addHandler(&ws);
  controlInterfaceWebServer.addHandler(&audio);
  controlInterfaceWebServer.begin();

  Devices::USB::CDC.setCallback(HostCommand::WSDATARECV, [](uint8_t *buffer, const size_t size) -> void
  { 
    ws.binaryAll(buffer, size);
  });

  Devices::Mic.setCallback([](uint8_t *buffer, const size_t size) -> bool
  {
    if (ESP.getFreeHeap() < (size * 2)) // double what we need
    {
      Debug::Log.info(LOG_WEB, "Could not write data to /audio, not enough free mem");
      return true;
    }

    audio.binaryAll(buffer, size);
    return true;
  });

  ElegantOTA.begin(&controlInterfaceWebServer); // Start ElegantOTA
}

void WebSite::loop(Preferences &prefs)
{
  if (preferences == nullptr)
  {
    return;
  }

  ws.cleanupClients();
  audio.cleanupClients();
  ElegantOTA.loop();
}

void WebSite::end()
{
  if (preferences == nullptr)
  {
    return;
  }

  // Not removing handlers etc is likely to leak memory
  // However we seem to get crashes if we do :(
  // TODO for later
}

#endif