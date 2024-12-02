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
 *  Copyright (c) 2010 Adriano dos Santos Fernandes <adrianosf@gmail.com>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#ifndef DSQL_BOOL_NODES_H
#define DSQL_BOOL_NODES_H

#include "firebird/impl/blr.h"
#include "../dsql/Nodes.h"

namespace Jrd {

class SubQuery;


class BinaryBoolNode final : public TypedNode<BoolExprNode, ExprNode::TYPE_BINARY_BOOL>
{
public:
	BinaryBoolNode(MemoryPool& pool, UCHAR aBlrOp, BoolExprNode* aArg1 = NULL,
		BoolExprNode* aArg2 = NULL);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	virtual void getChildren(NodeRefsHolder& holder, bool dsql) const
	{
		BoolExprNode::getChildren(holder, dsql);

		holder.add(arg1);
		holder.add(arg2);
	}

	virtual Firebird::string internalPrint(NodePrinter& printer) const;
	virtual BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch);
	virtual void genBlr(DsqlCompilerScratch* dsqlScratch);

	virtual bool ignoreNulls(const StreamList& streams) const
	{
		return (blrOp == blr_or) ?
			arg1->ignoreNulls(streams) && arg2->ignoreNulls(streams) :
			BoolExprNode::ignoreNulls(streams);
	}

	virtual BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const;
	bool dsqlMatch(DsqlCompilerScratch* dsqlScratch, const ExprNode* other, bool ignoreMapCast) const override;
	virtual bool sameAs(const ExprNode* other, bool ignoreStreams) const;
	virtual bool execute(thread_db* tdbb, Request* request) const;

	void ReplaceExpr(DsqlCompilerScratch* dsqlScratch, ValueExprNode* matchNode, ReplaceNodeFunc replaceNode) override
	{

	}

private:
	virtual bool executeAnd(thread_db* tdbb, Request* request) const;
	virtual bool executeOr(thread_db* tdbb, Request* request) const;

public:
	UCHAR blrOp;
	NestConst<BoolExprNode> arg1;
	NestConst<BoolExprNode> arg2;
};


class ComparativeBoolNode final : public TypedNode<BoolExprNode, ExprNode::TYPE_COMPARATIVE_BOOL>
{
public:
	enum DsqlFlag : UCHAR
	{
		DFLAG_NONE,
		DFLAG_ANSI_ALL,
		DFLAG_ANSI_ANY
	};

	ComparativeBoolNode(MemoryPool& pool, UCHAR aBlrOp, ValueExprNode* aArg1 = nullptr,
		ValueExprNode* aArg2 = nullptr, ValueExprNode* aArg3 = nullptr);

	ComparativeBoolNode(MemoryPool& pool, UCHAR aBlrOp, ValueExprNode* aArg1,
		DsqlFlag aDsqlFlag, ExprNode* aSpecialArg);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	virtual void getChildren(NodeRefsHolder& holder, bool dsql) const
	{
		BoolExprNode::getChildren(holder, dsql);

		holder.add(arg1);
		holder.add(arg2);
		holder.add(arg3);
		holder.add(dsqlSpecialArg);
	}

	virtual Firebird::string internalPrint(NodePrinter& printer) const;
	virtual BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch);
	virtual void genBlr(DsqlCompilerScratch* dsqlScratch);

	virtual bool possiblyUnknown() const
	{
		return (blrOp == blr_equiv) ? true : BoolExprNode::possiblyUnknown();
	}

	virtual bool ignoreNulls(const StreamList& streams) const
	{
		return (blrOp == blr_equiv) ? false : BoolExprNode::ignoreNulls(streams);
	}

	virtual BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const;
	bool dsqlMatch(DsqlCompilerScratch* dsqlScratch, const ExprNode* other, bool ignoreMapCast) const override;
	virtual bool sameAs(const ExprNode* other, bool ignoreStreams) const;
	virtual BoolExprNode* pass1(thread_db* tdbb, CompilerScratch* csb);
	virtual void pass2Boolean(thread_db* tdbb, CompilerScratch* csb, std::function<void ()> process);
	virtual bool execute(thread_db* tdbb, Request* request) const;

	void ReplaceExpr(DsqlCompilerScratch* dsqlScratch, ValueExprNode* matchNode, ReplaceNodeFunc replaceNode) override
	{
		if (this->arg1)
		{
			this->arg1 = this->arg1->findAndReplaceExpr(dsqlScratch, matchNode, replaceNode);
			fb_assert(this->arg1);
		}
		if (this->arg2)
		{
			this->arg2 = this->arg2->findAndReplaceExpr(dsqlScratch, matchNode, replaceNode);
			fb_assert(this->arg2);
		}
		if (this->arg3)
		{
			this->arg3 = this->arg3->findAndReplaceExpr(dsqlScratch, matchNode, replaceNode);
			fb_assert(this->arg3);
		}
	}

private:
	bool stringBoolean(thread_db* tdbb, Request* request, dsc* desc1, dsc* desc2,
		bool computedInvariant) const;
	bool sleuth(thread_db* tdbb, Request* request, const dsc* desc1, const dsc* desc2) const;

	BoolExprNode* createRseNode(DsqlCompilerScratch* dsqlScratch, UCHAR rseBlrOp);

public:
	UCHAR blrOp;
	bool dsqlCheckBoolean;
	DsqlFlag dsqlFlag;
	NestConst<ValueExprNode> arg1;
	NestConst<ValueExprNode> arg2;
	NestConst<ValueExprNode> arg3;
	NestConst<ExprNode> dsqlSpecialArg;	// list or select expression
};


class InListBoolNode : public TypedNode<BoolExprNode, ExprNode::TYPE_IN_LIST_BOOL>
{
	const static UCHAR blrOp = blr_in_list;

public:
	InListBoolNode(MemoryPool& pool, ValueExprNode* aArg = nullptr, ValueListNode* aList = nullptr);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	void getChildren(NodeRefsHolder& holder, bool dsql) const override
	{
		BoolExprNode::getChildren(holder, dsql);

		holder.add(arg);
		holder.add(list);
	}

	Firebird::string internalPrint(NodePrinter& printer) const override;
	BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch) override;
	void genBlr(DsqlCompilerScratch* dsqlScratch) override;

	bool ignoreNulls(const StreamList& streams) const override
	{
		// <arg> IN (<list>) is logically the same as <arg> = <list>[0] OR <arg> = <list>[1] OR ..
		// See above (BinaryBoolNode) the rule for the OR predicate: all its arguments should have
		// ignoreNulls == true to make the final result also true. Follow the same logic here.

		if (arg->ignoreNulls(streams))
			return true;

		for (const auto item : list->items)
		{
			if (!item->ignoreNulls(streams))
				return false;
		}

		return true;
	}

	BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const override;
	bool dsqlMatch(DsqlCompilerScratch* dsqlScratch, const ExprNode* other, bool ignoreMapCast) const override;
	bool sameAs(const ExprNode* other, bool ignoreStreams) const override;
	BoolExprNode* pass1(thread_db* tdbb, CompilerScratch* csb) override;
	void pass2Boolean(thread_db* tdbb, CompilerScratch* csb, std::function<void ()> process) override;
	bool execute(thread_db* tdbb, Request* request) const override;

public:
	NestConst<ValueExprNode> arg;
	NestConst<ValueListNode> list;
	NestConst<LookupValueList> lookup;
};


class MissingBoolNode final : public TypedNode<BoolExprNode, ExprNode::TYPE_MISSING_BOOL>
{
public:
	explicit MissingBoolNode(MemoryPool& pool, ValueExprNode* aArg = NULL, bool aDsqlUnknown = false);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	virtual void getChildren(NodeRefsHolder& holder, bool dsql) const
	{
		BoolExprNode::getChildren(holder, dsql);
		holder.add(arg);
	}

	virtual Firebird::string internalPrint(NodePrinter& printer) const;
	virtual BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch);
	virtual void genBlr(DsqlCompilerScratch* dsqlScratch);

	virtual bool possiblyUnknown() const
	{
		return true;
	}

	virtual bool ignoreNulls(const StreamList& /*streams*/) const
	{
		return false;
	}

	virtual BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const;
	virtual BoolExprNode* pass1(thread_db* tdbb, CompilerScratch* csb);
	virtual void pass2Boolean(thread_db* tdbb, CompilerScratch* csb, std::function<void ()> process);
	virtual bool execute(thread_db* tdbb, Request* request) const;
	void ReplaceExpr(DsqlCompilerScratch* dsqlScratch, ValueExprNode* matchNode, ReplaceNodeFunc replaceNode) override
	{
		if (this->arg)
		{
			this->arg = this->arg->findAndReplaceExpr(dsqlScratch, matchNode, replaceNode);
			fb_assert(this->arg);
		}
	}

public:
	bool dsqlUnknown;
	NestConst<ValueExprNode> arg;
};


class NotBoolNode final : public TypedNode<BoolExprNode, ExprNode::TYPE_NOT_BOOL>
{
public:
	explicit NotBoolNode(MemoryPool& pool, BoolExprNode* aArg = NULL);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	virtual void getChildren(NodeRefsHolder& holder, bool dsql) const
	{
		BoolExprNode::getChildren(holder, dsql);
		holder.add(arg);
	}

	virtual Firebird::string internalPrint(NodePrinter& printer) const;
	virtual BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch);
	virtual void genBlr(DsqlCompilerScratch* dsqlScratch);

	virtual BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const;
	virtual BoolExprNode* pass1(thread_db* tdbb, CompilerScratch* csb);
	virtual bool execute(thread_db* tdbb, Request* request) const;

	void ReplaceExpr(DsqlCompilerScratch* dsqlScratch, ValueExprNode* matchNode, ReplaceNodeFunc replaceNode) override
	{
		//
	}

private:
	BoolExprNode* process(DsqlCompilerScratch* dsqlScratch, bool invert);

public:
	NestConst<BoolExprNode> arg;
};


class RseBoolNode final : public TypedNode<BoolExprNode, ExprNode::TYPE_RSE_BOOL>
{
public:
	RseBoolNode(MemoryPool& pool, UCHAR aBlrOp, RecordSourceNode* aDsqlRse = nullptr);

	static DmlNode* parse(thread_db* tdbb, MemoryPool& pool, CompilerScratch* csb, const UCHAR blrOp);

	virtual void getChildren(NodeRefsHolder& holder, bool dsql) const
	{
		BoolExprNode::getChildren(holder, dsql);

		if (dsql)
			holder.add(dsqlRse);
		else
			holder.add(rse);
	}

	virtual Firebird::string internalPrint(NodePrinter& printer) const;
	virtual BoolExprNode* dsqlPass(DsqlCompilerScratch* dsqlScratch);
	virtual void genBlr(DsqlCompilerScratch* dsqlScratch);

	virtual bool dsqlAggregateFinder(AggregateFinder& visitor)
	{
		return visitor.ignoreSubSelects ? false : BoolExprNode::dsqlAggregateFinder(visitor);
	}

	virtual bool dsqlSubSelectFinder(SubSelectFinder& /*visitor*/)
	{
		return true;
	}

	virtual bool possiblyUnknown() const
	{
		return true;
	}

	virtual bool ignoreNulls(const StreamList& /*streams*/) const
	{
		return false;
	}

	virtual BoolExprNode* copy(thread_db* tdbb, NodeCopier& copier) const;
	bool dsqlMatch(DsqlCompilerScratch* dsqlScratch, const ExprNode* other, bool ignoreMapCast) const override;
	virtual bool sameAs(const ExprNode* other, bool ignoreStreams) const;
	virtual BoolExprNode* pass1(thread_db* tdbb, CompilerScratch* csb);
	virtual void pass2Boolean(thread_db* tdbb, CompilerScratch* csb, std::function<void ()> process);
	virtual bool execute(thread_db* tdbb, Request* request) const;
	void ReplaceExpr(DsqlCompilerScratch* dsqlScratch, ValueExprNode* matchNode, ReplaceNodeFunc replaceNode) override
	{
		//
	}

private:
	BoolExprNode* convertNeqAllToNotAny(thread_db* tdbb, CompilerScratch* csb);

public:
	UCHAR blrOp;
	bool ownSavepoint;
	NestConst<RecordSourceNode> dsqlRse;
	NestConst<RseNode> rse;
	NestConst<SubQuery> subQuery;
};


} // namespace

#endif // DSQL_BOOL_NODES_H
