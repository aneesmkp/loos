/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2016, Tod D. Romo, Alan Grossfield
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



#include <MultiTraj.hpp>

namespace loos {


  void MultiTrajectory::findNextUsableTraj() {
    for (; _curtraj < _trajectories.size(); ++_curtraj)
        if (_trajectories[_curtraj]->nframes() > _skip)
            break;
  }

  //! Rewinds MultiTrajectory and all contained trajectories
  void MultiTrajectory::rewindImpl() {

    for (uint i=0; i<_trajectories.size(); ++i)
      _trajectories[i]->rewind();
    _curtraj = 0;
    _curframe = skip;
    findNextUsableTraj();
    if (!eof())
        _trajectories[_curtraj]->readFrame(_curframe);
  }


  MultiTrajectory::Location MultiTrajectory::frameIndexToLocation(const uint i) {
    uint k, j;
    for (j=k=0; k<_trajectories.size(); ++k) {
      uint n = nframes(k);
      if (j + n > i)
        break;
      j += n;
    }
    return Location(k, (_skip + (i-j) * _stride));
  }
  
  
   void MultiTrajectory::seekNextFrameImpl() {
    if (eof())
        return;
    _curframe += _stride;
    if (_curframe >= _trajectories[_curtraj]->nframes()) {
      _curframe = _skip;
      ++_curtraj;
      findNextUsableTraj();
    }
  }

   void MultiTrajectory::seekFrameImpl(const uint i) {
    if (i >= _nframes)
        throw(FileReadError("Cannot seek past end of MultiTraj"));
     Location idx = frameIndexToLocation(i);
     _curtraj = idx.first;
     _curframe = idx.second;
  }

   bool MultiTrajectory::parseFrame() {
    if (eof())
        return 0;
    return(_trajectories[_curtraj]->readFrame(_curframe));
  }
    
   void MultiTrajectory::updateGroupCoordsImpl(AtomicGroup& g) {
     if (!eof())
        _trajectories[_curtraj]->updateGroupCoords(g);
  }


  void MultiTrajectory::initWithList(const std::vector<std::string>& filenames, const AtomicGroup& model) {
    for (uint i=0; i<filenames.size(); ++i) {
      pTraj traj = createTrajectory(filenames[i], model);
      _trajectories.push_back(traj);
      _nframes += nframes(i);
    }
  }

}

