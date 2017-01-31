create table notifications
(
    n_id int unsigned NOT NULL AUTO_INCREMENT primary key,
    n_time datetime NOT NULL,
    n_from varchar(255) NOT NULL,
    n_msg varchar(255) NOT NULL,
    n_type tinyint NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


create table currency (
  c_id int(10) unsigned NOT NULL AUTO_INCREMENT,
  c_type tinyint(4) NOT NULL DEFAULT '0',
  c_rate float NOT NULL,
  c_date date NOT NULL,
  c_time time NOT NULL,
  c_count int(10) unsigned NOT NULL,
  PRIMARY KEY (c_id),
  UNIQUE KEY uniq_curr (c_type, c_rate, c_date, c_time)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
