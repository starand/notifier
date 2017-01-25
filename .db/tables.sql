create table notifications
(
    n_id int unsigned NOT NULL AUTO_INCREMENT primary key,
    n_time datetime NOT NULL,
    n_from varchar(255) NOT NULL,
    n_msg varchar(255) NOT NULL,
    n_type tinyint NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


