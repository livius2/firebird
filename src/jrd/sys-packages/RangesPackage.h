/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Adriano dos Santos Fernandes
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2023 Karol Bieniaszewski <liviuslivius@poczta.onet.pl>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#ifndef JRD_SYS_PACKAGES_RANGES_PACKAGE_H
#define JRD_SYS_PACKAGES_RANGES_PACKAGE_H

#include "firebird.h"
#include "firebird/Message.h"
#include "../common/classes/array.h"
#include "../jrd/SystemPackages.h"

namespace Jrd {


class RangesPackage final : public SystemPackage
{
public:
	RangesPackage(Firebird::MemoryPool& pool);

	RangesPackage(const RangesPackage&) = delete;
	RangesPackage& operator=(const RangesPackage&) = delete;

private:
	FB_MESSAGE(NumbersInput, Firebird::ThrowStatusExceptionWrapper,
		(FB_INTEGER, numberFrom)
		(FB_INTEGER, numberTo)
	);

	FB_MESSAGE(NumbersOutput, Firebird::ThrowStatusExceptionWrapper,
		(FB_INTEGER, number)
	);

	class NumbersResultSet :
		public
			Firebird::DisposeIface<
				Firebird::IExternalResultSetImpl<
					NumbersResultSet,
					Firebird::ThrowStatusExceptionWrapper
				>
			>
	{
	public:
		NumbersResultSet(Firebird::ThrowStatusExceptionWrapper* status, Firebird::IExternalContext* context,
			const NumbersInput::Type* in, NumbersOutput::Type* out);

	public:
		void dispose() override
		{
			delete this;
		}

	public:
		FB_BOOLEAN fetch(Firebird::ThrowStatusExceptionWrapper* status) override;

	private:
		NumbersOutput::Type* out;
		Firebird::Array<NumbersOutput::Type> resultEntries{*getDefaultMemoryPool()};
		Firebird::Array<NumbersOutput::Type>::const_iterator resultIterator = nullptr;
	};

	//----------

	static Firebird::IExternalResultSet* numbersProcedure(Firebird::ThrowStatusExceptionWrapper* status,
		Firebird::IExternalContext* context, const NumbersInput::Type* in, NumbersOutput::Type* out);

};


}	// namespace

#endif	// JRD_SYS_PACKAGES_RANGES_PACKAGE_H
