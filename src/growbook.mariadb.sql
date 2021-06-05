BEGIN;

CREATE TABLE IF NOT EXISTS breeder (
	id SERIAL PRIMARY KEY,
	name VARCHAR(256) UNIQUE NOT NULL,
	homepage VARCHAR(1024) DEFAULT ''
);

CREATE INDEX IF NOT EXISTS idx_breeder_name ON breeder (name);

CREATE TABLE IF NOT EXISTS strain (
	id SERIAL PRIMARY KEY,
	breeder BIGINT UNSIGNED NOT NULL,
	name VARCHAR(256) NOT NULL,
	info MEDIUMTEXT,
	description MEDIUMTEXT,
	homepage VARCHAR(1024),
	seedfinder VARCHAR(1024),
	UNIQUE (breeder,name),
	FOREIGN KEY (breeder) REFERENCES breeder(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT	
);
CREATE INDEX IF NOT EXISTS idx_strain_breeder ON strain (breeder);
CREATE INDEX IF NOT EXISTS idx_strain_name ON strain (name); 

CREATE TABLE IF NOT EXISTS growlog (
	id SERIAL PRIMARY KEY,
	title VARCHAR(512) UNIQUE NOT NULL,
	description MEDIUMTEXT,
	created_on TIMESTAMP NOT NULL,
	flower_on DATE,
	finished_on TIMESTAMP
);
CREATE INDEX IF NOT EXISTS idx_growlog_title ON growlog(title);

CREATE TABLE IF NOT EXISTS growlog_entry (
	id SERIAL PRIMARY KEY,
	growlog BIGINT UNSIGNED NOT NULL,
	entry MEDIUMTEXT NOT NULL,
	created_on TIMESTAMP NOT NULL,
	FOREIGN KEY (growlog) REFERENCES growlog(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT
);
CREATE INDEX IF NOT EXISTS idx_growlog_entry_growlog ON growlog_entry(growlog);

CREATE TABLE IF NOT EXISTS growlog_strain (
	id SERIAL PRIMARY KEY,
	growlog BIGINT UNSIGNED NOT NULL,
	strain BIGINT USIGNED NOT NULL,
	UNIQUE(growlog,strain),
	FOREIGN KEY (growlog) REFERENCES growlog(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT,
	FOREIGN KEY (strain) REFERENCES strain(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT
);
CREATE INDEX IF NOT EXISTS idx_growlog_strain_growlog ON growlog_strain(growlog);
CREATE INDEX IF NOT EXISTS idx_growlog_strain_strain ON growlog_strain(strain);

COMMIT;
