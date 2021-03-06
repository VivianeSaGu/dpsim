/* Copyright 2017-2020 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#pragma once

#include <cps/Definitions.h>

namespace CPS {
namespace Base {
namespace Ph1 {
	class Inductor {
	protected:
		/// Inductance [H]
		Real mInductance;
	public:
		/// Sets model specific parameters
		void setParameters(Real inductance) {
			mInductance = inductance;
		}
	};
}
}
}
