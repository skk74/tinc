#include "tinc/TincServer.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ProcessorAsync.hpp"

#include <iostream>

using namespace tinc;

TincServer::TincServer() {}

void TincServer::onMessage(al::osc::Message &m) {

  if (m.addressPattern().substr(0, sizeof("/computationChains") - 1) ==
      "/computationChains") {
    if (m.typeTags()[0] == 'i') {
      int port;
      m >> port;
      for (auto *p : mProcessors) {
        al::osc::Packet oscPacket;
        std::cout << "Sending info on Processor " << p->id << " to "
                  << m.senderAddress() << ":" << port << std::endl;
        oscPacket.beginMessage("/registerProcessor");

        std::string type;
        if (strcmp(typeid(*p).name(), typeid(DataScript).name()) == 0) {
          type = "DataScript";
        } else if (strcmp(typeid(*p).name(), typeid(ComputationChain).name()) ==
                   0) {
          type = "ComputationChain";
        } else if (strcmp(typeid(*p).name(), typeid(CppProcessor).name()) ==
                   0) {
          type = "CppProcessor";
        }

        oscPacket << type << p->id << "";

        // FIXME we need to support multiple input and output files
        std::string inputFile;
        if (p->getInputFileNames().size() > 0) {
          inputFile = p->getInputFileNames()[0];
        }
        std::string outputFile;
        if (p->getOutputFileNames().size() > 0) {
          outputFile = p->getOutputFileNames()[0];
        }
        oscPacket << p->inputDirectory() << inputFile << p->outputDirectory()
                  << outputFile << p->runningDirectory();

        oscPacket.endMessage();

        al::osc::Send listenerRequest(port, m.senderAddress().c_str());
        listenerRequest.send(oscPacket);
        for (auto config : p->configuration) {

          oscPacket.clear();
          oscPacket.beginMessage("/processor/configuration");
          oscPacket << p->id << config.first;
          switch (config.second.type) {
          case FLAG_DOUBLE:
            oscPacket << config.second.flagValueDouble;
            break;
          case FLAG_INT:
            oscPacket << (int)config.second.flagValueInt;
            break;
          case FLAG_STRING:
            oscPacket << config.second.flagValueStr;
            break;
          }

          oscPacket.endMessage();

          listenerRequest.send(oscPacket);
        }

        if (dynamic_cast<ComputationChain *>(p)) {
          for (auto childProcessor :
               dynamic_cast<ComputationChain *>(p)->processors()) {

            if (strcmp(typeid(*childProcessor).name(),
                       typeid(ProcessorAsync).name()) == 0) {
              childProcessor =
                  dynamic_cast<ProcessorAsync *>(childProcessor)->processor();
            }
            if (strcmp(typeid(*childProcessor).name(),
                       typeid(DataScript).name()) == 0) {
              type = "DataScript";
            } else if (strcmp(typeid(*childProcessor).name(),
                              typeid(ComputationChain).name()) == 0) {
              type = "ComputationChain";
            } else if (strcmp(typeid(*childProcessor).name(),
                              typeid(CppProcessor).name()) == 0) {
              type = "CppProcessor";
            }
            oscPacket.clear();
            oscPacket.beginMessage("/registerProcessor");
            // FIXME we need to support multiple input and output files
            std::string inputFile;
            if (childProcessor->getInputFileNames().size() > 0) {
              inputFile = childProcessor->getInputFileNames()[0];
            }
            std::string outputFile;
            if (childProcessor->getOutputFileNames().size() > 0) {
              outputFile = childProcessor->getOutputFileNames()[0];
            }
            oscPacket << type << childProcessor->id << p->id
                      << childProcessor->inputDirectory() << inputFile
                      << childProcessor->outputDirectory() << outputFile
                      << childProcessor->runningDirectory();

            oscPacket.endMessage();

            listenerRequest.send(oscPacket);
            std::cout << "Sending info on Child Processor "
                      << childProcessor->id << std::endl;
            for (auto config : childProcessor->configuration) {

              oscPacket.clear();
              oscPacket.beginMessage("/processor/configuration");
              oscPacket << childProcessor->id << config.first;
              switch (config.second.type) {
              case FLAG_DOUBLE:
                oscPacket << config.second.flagValueDouble;
                break;
              case FLAG_INT:
                oscPacket << (int)config.second.flagValueInt;
                break;
              case FLAG_STRING:
                oscPacket << config.second.flagValueStr;
                break;
              }
              oscPacket.endMessage();
              listenerRequest.send(oscPacket);
            }
          }
        }
      }
    } else {
      std::cerr << "Unexpected syntax for /computationChains" << std::endl;
      m.print();
    }
    return;
  }

  if (m.addressPattern().substr(0, sizeof("/parameterSpaces") - 1) ==
      "/parameterSpaces") {
    if (m.typeTags()[0] == 'i') {
      int port;
      m >> port;
      for (auto *ps : mParameterSpaces) {
      }
    }
    return;
  }
}
