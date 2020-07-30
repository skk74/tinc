#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/ScriptProcessor.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include <fstream>
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Texture.hpp"

using namespace al;
using namespace tinc;

struct MyApp : public App {
    ParameterSpace ps;
    // CppProcessor processor;
    // Datascript object for python calls script_manager;
    ScriptProcessor processor;
    ControlGUI gui;

    std::string displayText;
    Texture graphTex;
    Texture graphTex2;

	std::string clean_double_to_string(double value){
		std::string val_as_string = std::to_string(value);
		auto trimmed = std::unique(val_as_string.begin(),val_as_string.end());
		std::string single_zero(val_as_string.begin(),trimmed);
		if (single_zero.back()=='0'){
		single_zero.pop_back();
		}
		if (single_zero.back()=='.'){
			single_zero.push_back('0');
		}
		return single_zero;
	}

    void defineParameterSpace() {
	auto eci1_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci1");
	auto eci2_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci2");
	auto eci3_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci3");
	auto eci4_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci4");

	// Create large parameter space
	std::vector<double> eci1_values = {-0.25, -0.125, 0.0, 0.125, 0.25};
	std::vector<double> eci2_values = {0.0, 0.5, 1.0, 1.5, 2.5};
	std::vector<double> eci3_values = {0.0, 0.5, 1.0, 1.5};
	std::vector<double> eci4_values = {0.0, 0.25, 0.5};
	for (const auto& val : eci1_values) {
	    eci1_dim->push_back(val, "_" + clean_double_to_string(val));
	}
	eci1_dim->conform();
	eci1_dim->type = ParameterSpaceDimension::MAPPED;
	for (const auto& val : eci2_values) {
	    eci2_dim->push_back(val, "_" + clean_double_to_string(val));
	}
	eci2_dim->type = ParameterSpaceDimension::MAPPED;
	eci2_dim->conform();
	for (const auto& val : eci3_values) {
	    eci3_dim->push_back(val, "_" + clean_double_to_string(val));
	}
	eci3_dim->type = ParameterSpaceDimension::MAPPED;
	eci3_dim->conform();
	for (const auto& val : eci4_values) {
	    eci4_dim->push_back(val, "_" + clean_double_to_string(val));
	}
	eci4_dim->type = ParameterSpaceDimension::MAPPED;
	eci4_dim->conform();

	ps.registerDimension(eci1_dim);
	ps.registerDimension(eci2_dim);
	ps.registerDimension(eci3_dim);
	ps.registerDimension(eci4_dim);

	// This function provided with a map of parameter name to index into
	// that parameter knows how to find the folder to run a process from
	ps.generateRelativeRunPath =
	    [&](std::map<std::string, size_t> indeces) {
		std::string path = "AMX2_spinel_diffusion_0.0_0.0";
		for (const auto& mapped_param : ps.dimensions) {
		    path += mapped_param->idAt(
			indeces[mapped_param->parameter().getName()]);
		}
		return path + "/";
	    };
	// Create necessary filesystem directories to be populated by data
	ps.createDataDirectories();

	// Register callback after every process call in a parameter sweep
	ps.onSweepProcess = [&](std::map<std::string, size_t> currentIndeces,
				double progress) {
	    std::cout << "Progress: " << progress * 100 << "%" << std::endl;
	};

	// Whenever the parameter space point changes, this function is called
	ps.registerChangeCallback(
	    [&](float value, ParameterSpaceDimension* changedDimension) {
		processor.setRunningDirectory(ps.currentRunPath());
		processor.process();
		Image img(ps.currentRunPath() + processor.getOutputFileNames()[0]);
		graphTex.resize(img.width(),
				img.height());
		graphTex.submit(img.array().data(), GL_RGBA,
				GL_UNSIGNED_BYTE);

		graphTex.filter(Texture::LINEAR);
		Image img2(ps.currentRunPath() + processor.getOutputFileNames()[1]);
		graphTex2.resize(img2.width(),
				img2.height());
		graphTex2.submit(img2.array().data(), GL_RGBA,
				GL_UNSIGNED_BYTE);

		graphTex2.filter(Texture::LINEAR);


		return true;
	    });
    }

    void initializeComputation() {
	// set local filename here relative to running directory and output
	// directory
	processor.setCommand("python");
	processor.setScriptName("grand_canonical/plot_voltage.py");
	processor.setOutputFileNames({"voltage_curve.png","site_occupancy.png"});
	// need to assign processor.registerDoneCallback to load image
	//(should be able to use running/output directory to find file)
	}
    void onInit() override {
	defineParameterSpace();
	initializeComputation();

	gui << ps.getDimension("eci1")->parameter();
	gui << ps.getDimension("eci2")->parameter();
	gui << ps.getDimension("eci3")->parameter();
	gui << ps.getDimension("eci4")->parameter();
	gui.init();

	// Now sweep the parameter space asynchronously
	ps.sweepAsync(processor);
    }
    void onCreate() override { graphTex.create2D(512, 512); }

    void onDraw(Graphics& g) override {
	g.clear(0);

    g.pushMatrix();
    g.translate(0, 0, -4);
	g.texture();
    g.quad(graphTex, -1.5, 1, 1.5, -1.5);
    g.popMatrix();
    g.pushMatrix();
    g.translate(0, 0, -4);
	g.texture();
    g.quad(graphTex2, 0.25, 1, 1.5, -1.5);
    g.popMatrix();


	gui.draw(g);
	//g.color(0);
	//FontRenderer::render(g, displayText.c_str(), {-1, 0, -4}, 0.1);
    }

    void onExit() override {
	ps.stopSweep();
	gui.cleanup();
    }
};

int main() {
    MyApp().start();

    return 0;
}
