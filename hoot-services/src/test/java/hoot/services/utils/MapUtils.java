package hoot.services.utils;

import static hoot.services.HootProperties.DB_NAME;
import static hoot.services.models.db.QMaps.maps;
import static hoot.services.models.db.QUsers.users;
import static hoot.services.utils.DbUtils.createQuery;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Calendar;

import org.springframework.transaction.annotation.Transactional;

import com.querydsl.core.types.dsl.Expressions;
import com.querydsl.sql.SQLQuery;

import hoot.services.models.db.QCurrentNodes;
import hoot.services.models.db.QCurrentRelationMembers;
import hoot.services.models.db.QCurrentRelations;
import hoot.services.models.db.QCurrentWayNodes;
import hoot.services.models.db.QCurrentWays;


public final class MapUtils {

    private MapUtils() {}

    /**
     *
     *
     * @param mapId

     *             //TODO: This code needs to be changed to dynamically read in
     *             the data types from querydsl. If I make a change to the
     *             schema in liquibase, it will never be picked up unless this
     *             static code is also changed. See r6777
     */
    @Transactional()
    public static void createMap(long mapId) {
        try {
            String dbname = DB_NAME;

            Connection connection = createQuery().getConnection();

            // changesets
            String createTblSql = "CREATE TABLE changesets_" + mapId + "(id bigserial NOT NULL, "
                    + " user_id bigint NOT NULL, " + " created_at timestamp without time zone NOT NULL, "
                    + " min_lat double precision NOT NULL, " + " max_lat double precision NOT NULL, "
                    + " min_lon double precision NOT NULL, " + " max_lon double precision NOT NULL, "
                    + " closed_at timestamp without time zone NOT NULL, " + " num_changes integer NOT NULL DEFAULT 0, "
                    + " tags hstore, " + " CONSTRAINT pk_changesets_" + mapId + " PRIMARY KEY (id ), "
                    + " CONSTRAINT changesets_" + mapId + "_user_id_fkey FOREIGN KEY (user_id) "
                    + " REFERENCES users (id) MATCH SIMPLE " + " ON UPDATE NO ACTION ON DELETE NO ACTION "
                    + " ) WITH ( OIDS=FALSE );";

            createTable(createTblSql, dbname, connection);

            // current_nodes
            createTblSql = "CREATE TABLE current_nodes_" + mapId + "(id bigserial NOT NULL, "
                    + " latitude double precision NOT NULL, " + " longitude double precision NOT NULL, "
                    + " changeset_id bigint NOT NULL, " + " visible boolean NOT NULL DEFAULT true, "
                    + " \"timestamp\" timestamp without time zone NOT NULL DEFAULT now(), " + " tile bigint NOT NULL, "
                    + " version bigint NOT NULL DEFAULT 1, " + " tags hstore, " + " CONSTRAINT current_nodes_" + mapId
                    + "_pkey" + " PRIMARY KEY (id ), " + " CONSTRAINT current_nodes_" + mapId
                    + "_changeset_id_fkey FOREIGN KEY (changeset_id) " + " REFERENCES changesets_" + mapId
                    + " (id) MATCH SIMPLE " + " ON UPDATE NO ACTION ON DELETE NO ACTION " + " ) WITH ( OIDS=FALSE );";

            createTable(createTblSql, dbname, connection);

            // current_relation_members
            createTblSql = "CREATE TABLE current_relation_members_" + mapId + "(relation_id bigint NOT NULL, "
                    + " member_type nwr_enum NOT NULL, " + " member_id bigint NOT NULL, "
                    + " member_role character varying(255) NOT NULL, " + " sequence_id integer NOT NULL DEFAULT 0, "
                    + " CONSTRAINT current_relation_members_" + mapId
                    + "_pkey PRIMARY KEY (relation_id , member_type , member_id , member_role , sequence_id ) "
                    + " ) WITH ( OIDS=FALSE );";

            createTable(createTblSql, dbname, connection);

            // current_relations
            createTblSql = "CREATE TABLE current_relations_" + mapId + "(" + "  id bigserial NOT NULL,"
                    + "  changeset_id bigint NOT NULL,"
                    + "  \"timestamp\" timestamp without time zone NOT NULL DEFAULT now(),"
                    + "  visible boolean NOT NULL DEFAULT true," + "  version bigint NOT NULL DEFAULT 1,"
                    + "  tags hstore," + "  CONSTRAINT current_relations_" + mapId + "_pkey PRIMARY KEY (id ),"
                    + "  CONSTRAINT current_relations_" + mapId + "_changeset_id_fkey FOREIGN KEY (changeset_id)"
                    + "      REFERENCES changesets_" + mapId + " (id) MATCH SIMPLE"
                    + "      ON UPDATE NO ACTION ON DELETE NO ACTION" + ")" + "WITH (" + "  OIDS=FALSE" + ");";

            createTable(createTblSql, dbname, connection);

            // current_way_nodes
            createTblSql = "CREATE TABLE current_way_nodes_" + mapId + "(" + "  way_id bigint NOT NULL,"
                    + "  node_id bigint NOT NULL," + "  sequence_id bigint NOT NULL," + "  CONSTRAINT current_way_"
                    + mapId + "_nodes_pkey PRIMARY KEY (way_id , sequence_id )" + ")" + "WITH (" + "  OIDS=FALSE"
                    + ");";

            createTable(createTblSql, dbname, connection);

            // current_ways
            createTblSql = "CREATE TABLE current_ways_" + mapId + "(" + "  id bigserial NOT NULL,"
                    + "  changeset_id bigint NOT NULL,"
                    + "  \"timestamp\" timestamp without time zone NOT NULL DEFAULT now(),"
                    + "  visible boolean NOT NULL DEFAULT true," + "  version bigint NOT NULL DEFAULT 1,"
                    + "  tags hstore," + "  CONSTRAINT current_ways_" + mapId + "_pkey PRIMARY KEY (id ),"
                    + "  CONSTRAINT current_ways_" + mapId + "_changeset_id_fkey FOREIGN KEY (changeset_id)"
                    + "      REFERENCES changesets_" + mapId + " (id) MATCH SIMPLE"
                    + "      ON UPDATE NO ACTION ON DELETE NO ACTION" + ")" + "WITH (" + "  OIDS=FALSE" + ");";

            createTable(createTblSql, dbname, connection);
        }
        catch (SQLException e) {
            throw new RuntimeException("Error creating map with id = " + mapId, e);
        }
    }

    /**
     * Determines whether any OSM element records exist in the services database
     *
     * @return true if any OSM element records exist in the services database;
     *         false otherwise
     */
    public static boolean elementDataExistsInServicesDb() {
        long recordCount = 0;

        SQLQuery query = createQuery().query();
        recordCount += query.from(QCurrentNodes.currentNodes).fetchCount();
        recordCount += query.from(QCurrentWayNodes.currentWayNodes).fetchCount();
        recordCount += query.from(QCurrentWays.currentWays).fetchCount();
        recordCount += query.from(QCurrentRelationMembers.currentRelationMembers).fetchCount();
        recordCount += query.from(QCurrentRelations.currentRelations).fetchCount();

        return (recordCount > 0);
    }

    public static long insertMap(long userId) {
        Long newId = createQuery()
                .select(Expressions.numberTemplate(Long.class, "nextval('maps_id_seq')"))
                .from()
                .fetchOne();

        if (newId != null) {
            Timestamp now = new Timestamp(Calendar.getInstance().getTimeInMillis());

            createQuery().insert(maps)
                    .columns(maps.id, maps.createdAt, maps.displayName, maps.publicCol, maps.userId)
                    .values(newId, now, "map-with-id-" + newId, true, userId)
                    .execute();
        }

        createMap(newId);

        return newId;
    }

    public static long insertUser() {
        Long newId = createQuery()
                .select(Expressions.numberTemplate(Long.class, "nextval('users_id_seq')"))
                .from()
                .fetchOne();

        if (newId != null) {
            createQuery().insert(users)
                    .columns(users.id, users.displayName, users.email)
                    .values(newId, "user-with-id-" + newId, "user-with-id-" + newId)
                    .execute();
        }

        return newId;
    }

    static void deleteUser(long userId) {
        createQuery().delete(users).where(users.id.eq(userId)).execute();
    }

    /**
     *
     * @param mapId
     */
    public static void deleteOSMRecord(Long mapId) {
        DbUtils.deleteMapRelatedTablesByMapId(mapId);
        createQuery().delete(maps).where(maps.id.eq(mapId)).execute();
    }

    private static void createTable(String createTblSql, String dbname, Connection conn) throws SQLException {
        try (PreparedStatement stmt = conn.prepareStatement(createTblSql)) {
            stmt.executeUpdate();
        }
    }
}
