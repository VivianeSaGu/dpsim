/**
 * @file
 * @author Junjie Zhang <junjie.zhang@eonerc.rwth-aachen.de>
 * @copyright 2017-2019, Institute for Automation of Complex Power Systems, EONERC
 *
 * CPowerSystems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <cps/SP/SP_Ph3_Inductor.h>

using namespace CPS;

SP::Ph3::Inductor::Inductor(String uid, String name, Logger::Level logLevel)
	: PowerComponent<Complex>(uid, name, logLevel) {
	mIntfVoltage = MatrixComp::Zero(3, 1);
	mIntfCurrent = MatrixComp::Zero(3, 1);
	setTerminalNumber(2);

	addAttribute<Matrix>("L", &mInductance, Flags::read | Flags::write);
}

PowerComponent<Complex>::Ptr SP::Ph3::Inductor::clone(String name) {
	auto copy = Inductor::make(name, mLogLevel);
	copy->setParameters(mInductance);
	return copy;
}

void SP::Ph3::Inductor::initializeFromPowerflow(Real frequency) {
	checkForUnconnectedTerminals();

	Real omega = 2 * PI * frequency;
	mSusceptance = MatrixComp::Zero(3, 3);
	mSusceptance <<
		Complex(0, -1 / omega / mInductance(0, 0)), Complex(0, -1 / omega / mInductance(0, 1)), Complex(0, -1 / omega / mInductance(0, 2)),
		Complex(0, -1 / omega / mInductance(1, 0)), Complex(0, -1 / omega / mInductance(1, 1)), Complex(0, -1 / omega / mInductance(1, 2)),
		Complex(0, -1 / omega / mInductance(2, 0)), Complex(0, -1 / omega / mInductance(2, 1)), Complex(0, -1 / omega / mInductance(2, 2));

	// IntfVoltage initialization for each phase
	mIntfVoltage(0, 0) = initialSingleVoltage(1) - initialSingleVoltage(0);
	mIntfVoltage(1, 0) = mIntfVoltage(0, 0) * Complex(cos(-2. / 3. * M_PI), sin(-2. / 3. * M_PI));
	mIntfVoltage(2, 0) = mIntfVoltage(0, 0) * Complex(cos(2. / 3. * M_PI), sin(2. / 3. * M_PI));
	mIntfCurrent = mSusceptance * mIntfVoltage;

	mSLog->info("--- Initialize according to power flow ---");
/*
	mLog.info() << "--- Initialize according to power flow ---" << std::endl
		<< "in phase A: " << std::endl
		<< "Voltage across: " << std::abs(mIntfVoltage(0, 0))
		<< "<" << Math::phaseDeg(mIntfVoltage(0, 0)) << std::endl
		<< "Current: " << std::abs(mIntfCurrent(0, 0))
		<< "<" << Math::phaseDeg(mIntfCurrent(0, 0)) << std::endl
		<< "Terminal 0 voltage: " << std::abs(initialSingleVoltage(0))
		<< "<" << Math::phaseDeg(initialSingleVoltage(0)) << std::endl
		<< "Terminal 1 voltage: " << std::abs(initialSingleVoltage(1))
		<< "<" << Math::phaseDeg(initialSingleVoltage(1)) << std::endl
		<< "--- Power flow initialization finished ---" << std::endl;
*/
}

void SP::Ph3::Inductor::mnaInitialize(Real omega, Real timeStep, Attribute<Matrix>::Ptr leftVector) {
	updateSimNodes();
// TODO 
/*
	mLog.info() << "Initial voltage " << Math::abs(mIntfVoltage(0, 0))
		<< "<" << Math::phaseDeg(mIntfVoltage(0, 0)) << std::endl
		<< "Initial current " << Math::abs(mIntfCurrent(0, 0))
		<< "<" << Math::phaseDeg(mIntfCurrent(0, 0)) << std::endl;
*/
	mMnaTasks.push_back(std::make_shared<MnaPostStep>(*this, leftVector));
	mRightVector = Matrix::Zero(leftVector->get().rows(), 1);
}

void SP::Ph3::Inductor::mnaApplySystemMatrixStamp(Matrix& systemMatrix) {
	if (terminalNotGrounded(0)) {
		// set upper left block, 3x3 entries
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(0, 0), mSusceptance(0, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(0, 1), mSusceptance(0, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(0, 2), mSusceptance(0, 2));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(0, 0), mSusceptance(1, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(0, 1), mSusceptance(1, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(0, 2), mSusceptance(1, 2));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(0, 0), mSusceptance(2, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(0, 1), mSusceptance(2, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(0, 2), mSusceptance(2, 2));
	}
	if (terminalNotGrounded(1)) {
		// set buttom right block, 3x3 entries
		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(1, 0), mSusceptance(0, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(1, 1), mSusceptance(0, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(1, 2), mSusceptance(0, 2));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(1, 0), mSusceptance(1, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(1, 1), mSusceptance(1, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(1, 2), mSusceptance(1, 2));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(1, 0), mSusceptance(2, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(1, 1), mSusceptance(2, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(1, 2), mSusceptance(2, 2));
	}
	// Set off diagonal blocks, 2x3x3 entries
	if (terminalNotGrounded(0) && terminalNotGrounded(1)) {
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(1, 0), -mSusceptance(0, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(1, 1), -mSusceptance(0, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 0), simNode(1, 2), -mSusceptance(0, 2));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(1, 0), -mSusceptance(1, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(1, 1), -mSusceptance(1, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 1), simNode(1, 2), -mSusceptance(1, 2));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(1, 0), -mSusceptance(2, 0));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(1, 1), -mSusceptance(2, 1));
		Math::addToMatrixElement(systemMatrix, simNode(0, 2), simNode(1, 2), -mSusceptance(2, 2));

		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(0, 0), -mSusceptance(0, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(0, 1), -mSusceptance(0, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 0), simNode(0, 2), -mSusceptance(0, 2));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(0, 0), -mSusceptance(1, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(0, 1), -mSusceptance(1, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 1), simNode(0, 2), -mSusceptance(1, 2));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(0, 0), -mSusceptance(2, 0));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(0, 1), -mSusceptance(2, 1));
		Math::addToMatrixElement(systemMatrix, simNode(1, 2), simNode(0, 2), -mSusceptance(2, 2));
	}

/*
	if (terminalNotGrounded(0))
		mLog.debug() << "Add " << mEquivCond << " to system at " << simNode(0) << "," << simNode(0) << std::endl;
	if (terminalNotGrounded(1))
		mLog.debug() << "Add " << mEquivCond << " to system at " << simNode(1) << "," << simNode(1) << std::endl;
	if (terminalNotGrounded(0) && terminalNotGrounded(1))
		mLog.debug() << "Add " << -mEquivCond << " to system at " << simNode(0) << "," << simNode(1) << std::endl
		<< "Add " << -mEquivCond << " to system at " << simNode(1) << "," << simNode(0) << std::endl;*/
}

void SP::Ph3::Inductor::MnaPostStep::execute(Real time, Int timeStepCount) {
	mInductor.mnaUpdateVoltage(*mLeftVector);
	mInductor.mnaUpdateCurrent(*mLeftVector);
}

void SP::Ph3::Inductor::mnaUpdateVoltage(const Matrix& leftVector) {
	// v1 - v0
	mIntfVoltage = Matrix::Zero(3, 1);
	if (terminalNotGrounded(1)) {
		mIntfVoltage(0, 0) = Math::complexFromVectorElement(leftVector, simNode(1, 0));
		mIntfVoltage(1, 0) = Math::complexFromVectorElement(leftVector, simNode(1, 1));
		mIntfVoltage(2, 0) = Math::complexFromVectorElement(leftVector, simNode(1, 2));
	}
	if (terminalNotGrounded(0)) {
		mIntfVoltage(0, 0) = mIntfVoltage(0, 0) - Math::complexFromVectorElement(leftVector, simNode(0, 0));
		mIntfVoltage(1, 0) = mIntfVoltage(1, 0) - Math::complexFromVectorElement(leftVector, simNode(0, 1));
		mIntfVoltage(2, 0) = mIntfVoltage(2, 0) - Math::complexFromVectorElement(leftVector, simNode(0, 2));
	}
}

void SP::Ph3::Inductor::mnaUpdateCurrent(const Matrix& leftVector) {
	mIntfCurrent = mSusceptance * mIntfVoltage;
}


// #### Tear Methods ####
void SP::Ph3::Inductor::mnaTearApplyMatrixStamp(Matrix& tearMatrix) {
	// TODO
	Math::addToMatrixElement(tearMatrix, mTearIdx, mTearIdx, 1. / mSusceptance(0, 0));
	Math::addToMatrixElement(tearMatrix, mTearIdx, mTearIdx, 1. / mSusceptance(1, 0));
	Math::addToMatrixElement(tearMatrix, mTearIdx, mTearIdx, 1. / mSusceptance(2, 0));
}

