export module IPostgresProcess;




namespace Postgres {

    export enum class postres_err_t  {
        POSTGRES_ERROR_OK = 0,
        POSTGRES_ERROR_FAILED_INIT_DB = 1,
    };


    export class PostgresProcessor {
    public:
        PostgresProcessor();
        ~PostgresProcessor();

        
        postres_err_t InitializeDatabaseConnection();
    };
}
