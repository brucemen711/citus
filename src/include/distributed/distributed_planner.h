/*-------------------------------------------------------------------------
 *
 * distributed_planner.h
 *	  General Citus planner code.
 *
 * Copyright (c) Citus Data, Inc.
 *-------------------------------------------------------------------------
 */

#ifndef DISTRIBUTED_PLANNER_H
#define DISTRIBUTED_PLANNER_H

#include "nodes/plannodes.h"

#if PG_VERSION_NUM >= 120000
#include "nodes/pathnodes.h"
#else
#include "nodes/relation.h"
#endif

#include "distributed/citus_nodes.h"
#include "distributed/errormessage.h"


/* values used by jobs and tasks which do not require identifiers */
#define INVALID_JOB_ID 0
#define INVALID_TASK_ID 0
#define MULTI_TASK_QUERY_INFO_OFF 0  /* do not log multi-task queries */

#define CURSOR_OPT_FORCE_DISTRIBUTED 0x080000

typedef struct RelationRestrictionContext
{
	bool hasDistributedRelation;
	bool hasLocalRelation;
	bool allReferenceTables;
	List *relationRestrictionList;
} RelationRestrictionContext;


typedef struct RootPlanParams
{
	PlannerInfo *root;

	/*
	 * Copy of root->plan_params. root->plan_params is not preserved in
	 * relation_restriction_equivalence, so we need to create a copy.
	 */
	List *plan_params;
} RootPlanParams;

typedef struct RelationRestriction
{
	Index index;
	Oid relationId;
	bool distributedRelation;
	RangeTblEntry *rte;
	RelOptInfo *relOptInfo;
	PlannerInfo *plannerInfo;
	List *prunedShardIntervalList;

	/* list of RootPlanParams for all outer nodes */
	List *outerPlanParamsList;
} RelationRestriction;

typedef struct JoinRestrictionContext
{
	List *joinRestrictionList;

	HTAB *intermediateResultJoins;
} JoinRestrictionContext;

typedef struct JoinRestriction
{
	JoinType joinType;
	List *joinRestrictInfoList;
	PlannerInfo *plannerInfo;
	RelOptInfo *innerrel;
	RelOptInfo *outerrel;
} JoinRestriction;



/* hash key */
typedef struct IntermediateResultHashKey
{
	char intermediate_result_id[NAMEDATALEN];
} IntermediateResultHashKey;

/* hash entry */
typedef struct IntermediateResultHashEntry
{
	IntermediateResultHashKey key;
	List *joinedRelationList;
} IntermediateResultHashEntry;

typedef struct PlannerRestrictionContext
{
	RelationRestrictionContext *relationRestrictionContext;
	JoinRestrictionContext *joinRestrictionContext;
	MemoryContext memoryContext;

} PlannerRestrictionContext;

typedef struct RelationShard
{
	CitusNode type;
	Oid relationId;
	uint64 shardId;
} RelationShard;

typedef struct RelationRowLock
{
	CitusNode type;
	Oid relationId;
	LockClauseStrength rowLockStrength;
} RelationRowLock;


extern PlannedStmt * distributed_planner(Query *parse, int cursorOptions,
										 ParamListInfo boundParams);
extern List * ExtractRangeTableEntryList(Query *query);
extern bool NeedsDistributedPlanning(Query *query);
extern struct DistributedPlan * GetDistributedPlan(CustomScan *node);
extern void multi_relation_restriction_hook(PlannerInfo *root, RelOptInfo *relOptInfo,
											Index index, RangeTblEntry *rte);
extern void multi_join_restriction_hook(PlannerInfo *root,
										RelOptInfo *joinrel,
										RelOptInfo *outerrel,
										RelOptInfo *innerrel,
										JoinType jointype,
										JoinPathExtraData *extra);
extern bool IsModifyCommand(Query *query);
extern bool IsModifyDistributedPlan(struct DistributedPlan *distributedPlan);
extern void EnsurePartitionTableNotReplicated(Oid relationId);
extern Node * ResolveExternalParams(Node *inputNode, ParamListInfo boundParams);
extern bool IsMultiTaskPlan(struct DistributedPlan *distributedPlan);
extern RangeTblEntry * RemoteScanRangeTableEntry(List *columnNameList);
extern int GetRTEIdentity(RangeTblEntry *rte);

#endif /* DISTRIBUTED_PLANNER_H */
