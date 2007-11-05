#include <sstream>
#include <string>

#include <TH1F.h>
#include <TCanvas.h>

#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "Geometry/CommonDetUnit/interface/GeomDetEnumerators.h"
#include "TrackingTools/DetLayers/interface/DetLayer.h"

#include "SimDataFormats/ValidationFormats/interface/MaterialAccountingStep.h"
#include "SimDataFormats/ValidationFormats/interface/MaterialAccountingDetector.h"
#include "MaterialAccountingLayer.h"

MaterialAccountingLayer::MaterialAccountingLayer( const DetLayer & layer ) :
  m_layer( & layer ),
  m_z( layer.surface().zSpan() ),
  m_r( layer.surface().rSpan() ),
  m_accounting(),
  m_errors(),
  m_tracks( 0 ),
  m_counted( false )
{
  m_dedx_spectrum   = new TH1F(0, "Energy loss spectrum", 1000,  0, 1);
  m_dedx_vs_eta     = new TH1F(0, "Energy loss vs. $eta", 1000, -8, 8);
  m_radlen_spectrum = new TH1F(0, "Radiation lengths spectrum", 1000,  0, 1);
  m_radlen_vs_eta   = new TH1F(0, "Radiation lengths vs. $eta", 1000, -8, 8);
}

MaterialAccountingLayer::~MaterialAccountingLayer(void)
{
  delete m_dedx_spectrum;
  delete m_dedx_vs_eta;
  delete m_radlen_spectrum;
  delete m_radlen_vs_eta;
}

bool MaterialAccountingLayer::addDetector( const MaterialAccountingDetector& detector ) {
  if (not inside(detector))
    return false;

  // multiple hits in the same layer (overlaps, etc.) from a single track still count as one for averaging,
  // since the energy deposits from the track have been already split between the different detectors
  m_buffer += detector.material();
  m_counted = true;

  return true;
}

void MaterialAccountingLayer::endOfTrack(void) {
  if (m_counted) {
    m_accounting += m_buffer;
    m_errors     += m_buffer * m_buffer;
    ++m_tracks;

    m_dedx_spectrum->Fill(   m_buffer.energyLoss() );
    m_dedx_vs_eta->Fill(     m_buffer.in().eta() );
    m_radlen_spectrum->Fill( m_buffer.radiationLengths() );
    m_radlen_vs_eta->Fill(   m_buffer.in().eta() );
  }
  m_counted = false;
  m_buffer  = MaterialAccountingStep();
}

std::string MaterialAccountingLayer::getName(void) const {
  // extract the subdetector name using the overloaded operator<<
  std::stringstream name;
  name << m_layer->subDetector();
  return name.str();
}


void MaterialAccountingLayer::savePlots(const std::string & name) const
{
  TCanvas * canvas;
  double integral;

  canvas = new TCanvas ("cavas", "Energy loss", 800, 600);
  m_dedx_spectrum->Draw("");
  canvas->SaveAs((name + "_dedx_spectrum.png").c_str(),  "");
  canvas->SaveAs((name + "_dedx_spectrum.root").c_str(), "");
  delete canvas;
  
  canvas = new TCanvas ("canvas", "Energy loss vs. $eta", 800, 600);
  integral = m_dedx_vs_eta->Integral();
  if (integral != 0) {
    for (unsigned int i = 0 ; i < m_dedx_vs_eta->GetSize(); ++i)
      (*m_dedx_vs_eta)[i] /= integral;
  }
  m_dedx_vs_eta->Draw("");
  canvas->SaveAs((name + "_dedx_vs_eta.png").c_str(),  "");
  canvas->SaveAs((name + "_dedx_vs_eta.root").c_str(), "");
  delete canvas;
  
  canvas = new TCanvas ("canvas", "Radiation lenghts", 800, 600);
  m_radlen_spectrum->Draw("");
  canvas->SaveAs((name + "_radlen_spectrum.png").c_str(),  "");
  canvas->SaveAs((name + "_radlen_spectrum.root").c_str(), "");
  delete canvas;
  
  canvas = new TCanvas ("canvas", "Radiation lenghts vs $eta", 800, 600);
  integral = m_radlen_vs_eta->Integral();
  if (integral != 0) {
    for (unsigned int i = 0 ; i < m_radlen_vs_eta->GetSize(); ++i)
      (*m_radlen_vs_eta)[i] /= integral;
  }
  m_radlen_vs_eta->Draw("");
  canvas->SaveAs((name + "_radlen_vs_eta.png").c_str(),  "");
  canvas->SaveAs((name + "_radlen_vs_eta.root").c_str(), "");
  delete canvas;
}

