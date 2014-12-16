/*
  aligner.cpp


  Aligns structures in a trajectory...

*/




/*

  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <loos.hpp>

using namespace std;
using namespace loos;

namespace opts = loos::OptionsFramework;
namespace po = loos::OptionsFramework::po;

const uint min_align_selection_warning = 7;   // Warn user when selecting fewer than this # of atoms
                                              // This number has not been rigorously determined...
   



// @cond TOOLS_INTERNAL
string fullHelpMessage(void) {
  string msg =
    "\n"
    "SYNOPSIS\n"
    "Aligns the structures in a trajectory\n"
    "\n"
    "DESCRIPTION\n"
    "\taligner can align a trajectory two different ways: an optimal alignment of all structures,\n"
    "or aligning all structures to a reference structure.  The optimal alignment uses an iterative\n"
    "algorithm (described in Grossfield, et al. Proteins 67, 31–40 (2007)).  The latter method\n"
    "uses a Kabsch least-squares algorithm to superimpose each frame of the trajectory onto the\n"
    "reference structure.\n"
    "\taligner can take multiple selections to govern what is aligned.  The --align option\n"
    "determines what subset of atoms from the trajectory is used in computing the alignment\n"
    "transformation for each frame.  The --transform option determines which atoms from the\n"
    "trajectory the alignment transformation is applied to.  Only these atoms are written out\n"
    "to the DCD file generated by aligner.  Note that the default --transform selection is 'all',\n"
    "so if you want to align alpha-carbons and only output alpha-carbons, you will need to provide\n"
    "the same selection (i.e. \"name == 'CA'\") to both options.\n"
    "\tWhen aligning to a reference structure, the selection given in --align is used for\n"
    "both the trajectory and the reference structure.  Sometimes, however, the selection expression\n"
    "for the trajectory does not match the reference structure (e.g. aligning a trajectory to a\n"
    "crystal structure).  In this case, use the --refsel option to specify a different selection\n"
    "expression for the reference structure.  Note that the atoms must be in the same order as\n"
    "the ones picked by the trajectory --align (i.e. the first atom from the reference subset\n"
    "must match the first atom in the trajectory subset, etc).\n"
    "\tThe --xyonly and --noztrans options may be of interest for membrane systems.\n"
    "The --xyonly option will translate the alignment subset, but will only rotate about\n"
    "the Z-axis (i.e. within the x,y-plane).  The --noztrans option will remove any Z-translation\n"
    "in the subset after alignment.  For example, aligning with --xyonly turned on will orient\n"
    "the subset such that it is aligned within the x,y-plane, but the tilt relative to the x,y-plane\n"
    "is preserved.  However, the centroid of the selection will always be centered.  If the --noztrans\n"
    "option is turned on, then the selection will be centered in x and y, but the z-coordinates\n"
    "will be preserved.\n"
    "\n"
    "\n"
    "EXAMPLES\n"
    "\n"
    "\taligner --prefix b2ar-aligned b2ar.pdb b2ar.dcd\n"
    "Aligns the trajectory based on the default selection (alpha-carbons).  All atoms are transformed\n"
    "and the model is written to b2ar-aligned.pdb and the aligned trajectory is written to\n"
    "b2ar-aligned.dcd\n"
    "\n"
    "\taligner --prefix aligned --transform 'name =~ \"^(C|O|N|CA)$\"' model.pdb traj.dcd\n"
    "Aligns the trajectory based on the default selection (alpha-carbons).  Only backbone atoms are\n"
    "transformed.  Creates aligned.pdb and aligned.dcd\n"
    "\n"
    "\taligner --prefix aligned --align 'segid == \"HEME\"' \\\n"
    "\t  --transform 'segid == \"PROT\" || segid == \"HEME\"' model.pdb traj.dcd\n"
    "Aligns the trajectory based on atoms with segid 'HEME'.  Only transforms protein and heme atoms\n"
    "\n"
    "\taligner --prefix aligned --reference xtal.pdb model.pdb traj.dcd\n"
    "Aligns using alpha-carbons, transforming all atoms, but align the trajectory to the structure\n"
    "in xtal.pdb\n"
    "\n"
    "\taligner --prefix aligned --reference xtal.pdb \\\n"
    "\t  --refsel 'resid >= 30 && resid <= 60 && name == \"CA\"' \\\n"
    "\t  --align 'resid >= 10 && resid <= 40 && name == \"CA\"' model.pdb traj.dcd\n"
    "Aligns against a reference structure.  Uses alpha-carbons from residues 30-60 in the reference\n"
    "structure, aligned against alpha-carbons from residues 10-40 from the trajectory.  All atoms\n"
    "in the trajectory are transformed.\n"
    "\n"
    "NOTES\n"
    "\n"
    "\tSelecting too few atoms to align may result in a poor alignment.\n";

  return(msg);
}



class ToolOptions : public opts::OptionsPackage {
public:
  ToolOptions() : alignment_string("name == 'CA'"),
                  transform_string("all"),
                  reference_name(""),
                  reference_sel(""),
                  alignment_tol(1e-6),
                  maxiter(5000),
                  xy_only(false),
                  no_ztrans(false)
                  { }

    void addGeneric(po::options_description& o) {
        o.add_options()
            ("align", po::value<string>(&alignment_string)->default_value(alignment_string), "Align using this selection")
            ("transform", po::value<string>(&transform_string)->default_value(transform_string), "Transform using this selection")
            ("maxiter", po::value<uint>(&maxiter)->default_value(maxiter), "Maximum number of iterations for alignment algorith")
            ("tolerance", po::value<double>(&alignment_tol)->default_value(alignment_tol), "Tolerance for alignment convergence")
            ("reference", po::value<string>(&reference_name), "Align to a reference structure (non-iterative")
            ("refsel", po::value<string>(&reference_sel), "Selection to align against in reference (default is same as --align)")
            ("xyonly", po::value<bool>(&xy_only)->default_value(xy_only), "Only align in x and y (i.e. rotations about Z, but translated in x,y,z)")
            ("noztrans", po::value<bool>(&no_ztrans)->default_value(no_ztrans), "Do not translate selection in Z");
    }

  string print() const {
    ostringstream oss;
    oss << boost::format("align='%s',transform='%s',maxiter=%d,tolerance=%f,reference='%s',refsel='%s'")
      % alignment_string % transform_string
      % maxiter % alignment_tol
      % reference_name % reference_sel;
    return(oss.str());
  }


    string alignment_string, transform_string;
    string reference_name, reference_sel;
    double alignment_tol;
    uint maxiter;
    bool xy_only, no_ztrans;
};


class ReadFrame 
{
public:
    
    ReadFrame(const pTraj& trj) 
        : _traj(trj)
        {}


    virtual void read(const uint i, AtomicGroup& grp) 
        {
            _traj->readFrame(i);
            _traj->updateGroupCoords(grp);
        }

protected:
    pTraj _traj;
};


class XYReadFrame : public ReadFrame 
{
public:
    XYReadFrame(const pTraj& trj)
        : ReadFrame(trj)
        {}


    virtual void read(const uint i, AtomicGroup& grp) 
        {
            ReadFrame::read(i, grp);
            for (AtomicGroup::iterator j = grp.begin(); j != grp.end(); ++j)
                (*j)->coords().z() = 0.0;
        }

};


// @endcond



boost::tuple<vector<XForm>, greal, int> iterativeAlignment(const AtomicGroup& g,
                                                           pTraj& traj,
                                                           const std::vector<uint>& frame_indices,
                                                           greal threshold,
                                                           int maxiter,
                                                           ReadFrame* rfop) {

    // Must first prime the loop...
    AtomicGroup frame = g.copy();
    rfop->read(frame_indices[0], frame);
      
    uint nf = frame_indices.size();

    int iter = 0;
    greal rms;
    std::vector<XForm> xforms(nf);
    AtomicGroup avg = frame.copy();

    AtomicGroup target = frame.copy();
    target.centerAtOrigin();

    do {
        // Compute avg internally so we don't have to read traj twice...
        for (uint j=0; j<avg.size(); ++j)
            avg[j]->coords() = GCoord(0,0,0);
        
        for (uint i=0; i<nf; ++i) {

            rfop->read(frame_indices[i], frame);

            GMatrix M = frame.alignOnto(target);
            xforms[i].load(M);

            for (uint j=0; j<avg.size(); ++j)
                avg[j]->coords() += frame[j]->coords();
        }

        for (uint j=0; j<avg.size(); ++j)
            avg[j]->coords() /= nf;

        rms = avg.rmsd(target);
        target = avg.copy();
        ++iter;
    } while (rms > threshold && iter <= maxiter);

    boost::tuple<vector<XForm>, greal, int> res(xforms, rms, iter);
    return(res);
    
}








void savePDB(const string& fname, const string& meta, const AtomicGroup& grp) {
  AtomicGroup dup = grp.copy();
  PDB pdb = PDB::fromAtomicGroup(dup);
  pdb.pruneBonds();
  pdb.remarks().add(meta);
  ofstream ofs(fname.c_str());
  ofs << pdb;
}




int main(int argc, char *argv[]) {

  // Parse command-line options, cache invocation header for later use...
  string header = invocationHeader(argc, argv);
  opts::BasicOptions* bopts = new opts::BasicOptions(fullHelpMessage());
  opts::OutputPrefix* prefopts = new opts::OutputPrefix;
  opts::TrajectoryWithFrameIndices* tropts = new opts::TrajectoryWithFrameIndices;
  opts::OutputTrajectoryTypeOptions *otopts = new opts::OutputTrajectoryTypeOptions;
  ToolOptions* topts = new ToolOptions;

  opts::AggregateOptions options;
  options.add(bopts).add(prefopts).add(tropts).add(otopts).add(topts);
  if (!options.parse(argc, argv))
    exit(-1);

  // Read the inputs...
  AtomicGroup model = tropts->model;
  pTraj traj = tropts->trajectory;

  // Get the selections (subsets) to operate over
  AtomicGroup align_sub = selectAtoms(model, topts->alignment_string);
  if (align_sub.size() < min_align_selection_warning)
      cerr << "Warning- selecting fewer than " << min_align_selection_warning << " atoms with --align may result in a poor quality alignment.\n";

  
  AtomicGroup applyto_sub = selectAtoms(model, topts->transform_string);


  ReadFrame* reader;
  if (topts->xy_only)
      reader = new XYReadFrame(traj);
  else
      reader = new ReadFrame(traj);

  // Now do the alignin'...
  vector<uint> indices = tropts->frameList();
  unsigned int nframes = indices.size();

  if (topts->reference_name.empty()) {

    boost::tuple<vector<XForm>,greal, int> res = iterativeAlignment(align_sub, traj, indices, topts->alignment_tol, topts->maxiter, reader);
    greal final_rmsd = boost::get<1>(res);
    cerr << "Final RMSD between average structures is " << final_rmsd << endl;
    cerr << "Total iters = " << boost::get<2>(res) << endl;
    
    vector<XForm> xforms = boost::get<0>(res);
    
    // Setup for writing Trajectory
    pTrajectoryWriter outtraj = otopts->createTrajectory(prefopts->prefix);
    outtraj->setComments(header);
    
    // Now apply the alignment transformations to the requested subsets
    for (unsigned int i = 0; i<nframes; i++) {
      traj->readFrame(indices[i]);
      traj->updateGroupCoords(model);
      GCoord original_center;
      if (topts->no_ztrans)
          original_center = applyto_sub.centroid();
      
      model.applyTransform(xforms[i]);
      if (topts->no_ztrans) {
          GCoord new_center = applyto_sub.centroid();
          double dz = original_center.z() - new_center.z();
          GCoord shift(0.0, 0.0, dz);
          applyto_sub.translate(shift);
      }
      
      outtraj->writeFrame(applyto_sub);
      
      if (i == 0) 
        savePDB(prefopts->prefix + ".pdb", header, applyto_sub);
    }
    
  } else {
    
    AtomicGroup reference = createSystem(topts->reference_name);
    
    string refsel = topts->reference_sel.empty() ? topts->alignment_string : topts->reference_sel;
    AtomicGroup refsub = selectAtoms(reference, refsel);

    if (refsub.size() != align_sub.size()) {
      cerr << boost::format("ERROR- alignment subset has %d atoms but reference subset has %d.  They must match.\n") % align_sub.size() % refsub.size();
      exit(-10);
    }

    pTrajectoryWriter outtraj = otopts->createTrajectory(prefopts->prefix);
    outtraj->setComments(header);

    bool first = true;
    for (vector<uint>::iterator i = indices.begin(); i != indices.end(); ++i) {
      traj->readFrame(*i);
      traj->updateGroupCoords(model);

      GCoord original_center;
      if (topts->no_ztrans)
          original_center = applyto_sub.centroid();

      GMatrix M = align_sub.superposition(refsub);
      XForm W(M);
      applyto_sub.applyTransform(W);

      if (topts->no_ztrans) {
          GCoord new_center = applyto_sub.centroid();
          double dz = original_center.z() - new_center.z();
          GCoord shift(0.0, 0.0, dz);
          applyto_sub.translate(shift);
      }

      outtraj->writeFrame(applyto_sub);

      if (first) {
        savePDB(prefopts->prefix + ".pdb", header, applyto_sub);
        first = false;
      }
    }

  }
}



    
