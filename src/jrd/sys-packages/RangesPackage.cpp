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

#include "firebird.h"
#include "../jrd/sys-packages/RangesPackage.h"
#include "../dsql/DsqlRequests.h"
#include "../jrd/Statement.h"
#include "../jrd/recsrc/RecordSource.h"
#include "../dsql/dsql_proto.h"
#include "../jrd/mov_proto.h"

using namespace Jrd;
using namespace Firebird;


//--------------------------------------


IExternalResultSet* RangesPackage::numbersProcedure(ThrowStatusExceptionWrapper* status,
	IExternalContext* context, const NumbersInput::Type* in, NumbersOutput::Type* out)
{
	return FB_NEW NumbersResultSet(status, context, in, out);
}

//--------------------------------------


RangesPackage::NumbersResultSet::NumbersResultSet(ThrowStatusExceptionWrapper* status,
		IExternalContext* context, const NumbersInput::Type* in, NumbersOutput::Type* aOut)
	: out(aOut)
{
	if (!in->numberFromNull || !in->numberToNull || in->numberTo >= in->numberFrom)
	{
		for (auto i = in->numberFrom; i <= in->numberTo; i++)
		{
			auto& resultEntry = resultEntries.add();

			resultEntry.numberNull = FB_FALSE;
			resultEntry.number = i;
		}
	}

	resultIterator = resultEntries.begin();
}

FB_BOOLEAN RangesPackage::NumbersResultSet::fetch(ThrowStatusExceptionWrapper* status)
{
	if (resultIterator >= resultEntries.end())
		return false;

	*out = *resultIterator++;

	return true;
}

//--------------------------------------


RangesPackage::RangesPackage(MemoryPool& pool)
	: SystemPackage(
		pool,
		"RDB$RANGES",
		ODS_14_1,
		// procedures
		{
			SystemProcedure(
				pool,
				"NUMBERS",
				SystemProcedureFactory<NumbersInput, NumbersOutput, numbersProcedure>(),
				prc_selectable,
				// input parameters
				{
					{"RDB$NUMBER_FROM", fld_integer, false},
					{"RDB$NUMBER_TO", fld_integer, false}
				},
				// output parameters
				{
					{"RDB$NUMBER", fld_integer, false}
				}
			),
		},
		// functions
		{
		}
	)
{
}
