SET citus.next_shard_id TO 1670000;
CREATE SCHEMA expression_reference_join;
SET search_path TO expression_reference_join;
SET citus.shard_count TO 4;
SET citus.enable_repartition_joins TO on;

CREATE TABLE ref (a int, b int);
CREATE TABLE test (x int, y int);

INSERT INTO ref VALUES
    (2,2),
    (4,4);

INSERT INTO test VALUES
    (1,2),
    (2,2);

SELECT create_reference_table('ref');
SELECT create_distributed_table('test', 'x');

-- PR 3180 implements expressions in join clauses to reference tables to support CHbenCHmark queries 7/8/9
-- plannable as a repartition + reference join, now with an expression in the join clause
SELECT *
FROM
    test t1 JOIN test t2 USING (y), -- causes repartition, which makes this not routable or pushdownable
    ref a
WHERE t2.y * 2 = a.a
ORDER BY 1,2,3;

-- The join clause is wider than it used to be, causing this query to be recognized by the LogicalPlanner as a repartition join.
-- Unplannable query due to a three-way join which causes no valid path (besides the cartesian product) to be found
SELECT *
FROM
    test t1 JOIN test t2 USING (y), -- causes repartition, which makes this not routable or pushdownable
    ref a,
    ref b
WHERE t2.y - a.a - b.b = 0
ORDER BY 1,2,3;

SET client_min_messages TO WARNING;
DROP SCHEMA expression_reference_join CASCADE;
