BEGIN TRANSACTION;

PRAGMA encoding='UTF-8'; 

CREATE TABLE IF NOT EXISTS breeder (
	id INTEGER PRIMARY KEY,
	name VARCHAR(256) UNIQUE NOT NULL,
	homepage VARCHAR(1024) DEFAULT ''
);

CREATE TABLE IF NOT EXISTS strain (
	id INTEGER PRIMARY KEY,
	breeder INTEGER NOT NULL,
	name VARCHAR(256) NOT NULL,
	info TEXT DEFAULT '',
	description TEXT DEFAULT '',
	homepage VARCHAR(1024) DEFAULT '',
	seedfinder VARCHAR(1024) DEFAULT '',
	UNIQUE (breeder,name),
	FOREIGN KEY (breeder) REFERENCES breeder(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT	
);
CREATE INDEX IF NOT EXISTS idx_strain_breeder ON strain (breeder);
CREATE INDEX IF NOT EXISTS idx_strain_name ON strain (name);

CREATE VIEW IF NOT EXISTS strain_view AS
	SELECT	t1.id AS id,
			t1.breeder AS breeder_id,
			t2.name AS breeder_name,
			t1.name AS name,
			t1.info AS info,
			t1.description AS description,
			t1.homepage AS homepage,
			t1.seedfinder AS seedfinder
		FROM strain AS t1 JOIN breeder AS t2
			ON t1.breeder = t2.id;


CREATE TABLE IF NOT EXISTS growlog (
	id INTEGER PRIMARY KEY,
	title VARCHAR(1024) UNIQUE NOT NULL,
	description TEXT DEFAULT '',
	created_on TIMESTAMP NOT NULL,
	flower_on DATE,
	finished_on TIMESTAMP
);
CREATE INDEX IF NOT EXISTS idx_growlog_title ON growlog(title);

CREATE TABLE IF NOT EXISTS growlog_entry (
	id INTEGER PRIMARY KEY,
	growlog INTEGER NOT NULL,
	entry TEXT NOT NULL,
	created_on TIMESTAMP NOT NULL,
	FOREIGN KEY (growlog) REFERENCES growlog(id)
		ON UPDATE CASCADE
		ON DELETE RESTRICT
);
CREATE INDEX IF NOT EXISTS idx_growlog_entry_growlog ON growlog_entry(growlog);

CREATE TABLE IF NOT EXISTS growlog_strain (
	id INTEGER PRIMARY KEY,
	growlog INTEGER NOT NULL,
	strain INTEGER NOT NULL,
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