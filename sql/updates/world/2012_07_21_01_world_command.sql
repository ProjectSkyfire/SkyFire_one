DELETE FROM command WHERE NAME IN ('reload spell_ranks');
INSERT INTO command (`name`, `security`, `help`) VALUES
('reload spell_ranks','3','Usage: .reload spell_ranks\r\nReloads the spell_ranks DB table.');
