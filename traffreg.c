#include "traffreg.h"
#include "common.h"
#include "database.h"

static inline pdd_traffregs_t *get_traffregs()
{
	static pdd_traffregs_t *traffregs = NULL;
	if (!traffregs)
	{
		traffregs = g_ptr_array_new();
	}
	return traffregs;
}

static inline GHashTable *get_traffregs_images()
{
	static GHashTable *traffregs_images = NULL;
	if (!traffregs_images)
	{
		traffregs_images = g_hash_table_new(NULL, NULL);
	}
	return traffregs_images;
}

pdd_traffreg_t *traffreg_new_with_id(gint64 id, gint32 number, const gchar *text)
{
	pdd_traffreg_t *traffreg = g_new(pdd_traffreg_t, 1);
	traffreg->id = id;
	traffreg->number = number;
	traffreg->text = g_strdup(text);
	return traffreg;
}

pdd_traffreg_t *traffreg_copy(pdd_traffreg_t *traffreg)
{
	return traffreg_new_with_id(traffreg->id, traffreg->number, traffreg->text);
}

pdd_traffreg_t *traffreg_new(gint32 number, const gchar *text)
{
	return traffreg_new_with_id(0, number, text);
}

void traffreg_free(pdd_traffreg_t *traffreg)
{
	g_free(traffreg->text);
	g_free(traffreg);
}

gboolean traffreg_save(pdd_traffreg_t *traffreg)
{
	if (!use_cache)
	{
		static gint64 id = 0;
		traffreg->id = ++id;
		g_ptr_array_add(get_traffregs(), traffreg_copy(traffreg));
		return TRUE;
	}

	static sqlite3_stmt *stmt = NULL;
	sqlite3 *db = database_get();
	int result;

	if (!stmt)
	{
		result = sqlite3_prepare_v2(db, "INSERT INTO `traffregs` (`number`, `text`) VALUES (?, ?)", -1, &stmt, NULL);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
	}

	result = sqlite3_reset(stmt);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_bind_int(stmt, 1, traffreg->number);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_bind_text(stmt, 2, traffreg->text, -1, NULL);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		g_error("traffreg: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
	}
	
	traffreg->id = sqlite3_last_insert_rowid(db);

	return TRUE;
}

gboolean traffreg_set_images(pdd_traffreg_t *traffreg, pdd_images_t *images)
{
	if (!use_cache)
	{
		g_hash_table_insert(get_traffregs_images(), traffreg, image_copy_all(images));
		return TRUE;
	}

	static sqlite3_stmt *stmt = NULL;
	sqlite3 *db = database_get();
	int result;

	if (!stmt)
	{
		result = sqlite3_prepare_v2(db, "INSERT INTO `images_traffregs` (`image_id`, `traffreg_id`) VALUES (?, ?)", -1, &stmt, NULL);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
	}

	gsize i;
	for (i = 0; i < images->len; i++)
	{
		pdd_image_t *image = ((pdd_image_t **)images->pdata)[i];

		result = sqlite3_reset(stmt);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}

		result = sqlite3_bind_int64(stmt, 1, image->id);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
		}

		result = sqlite3_bind_int64(stmt, 2, traffreg->id);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
		}

		result = sqlite3_step(stmt);
		if (result != SQLITE_DONE)
		{
			g_error("traffreg: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
	}

	return TRUE;
}

pdd_traffreg_t *traffreg_find_by_id(gint64 id)
{
	if (!use_cache)
	{
		gsize i;
		for (i = 0; i < get_traffregs()->len; i++)
		{
			pdd_traffreg_t *traffreg = g_ptr_array_index(get_traffregs(), i);
			if (traffreg->id == id)
			{
				return traffreg_copy(traffreg);
			}
		}
		return NULL;
	}

	static sqlite3_stmt *stmt = NULL;
	sqlite3 *db = database_get();
	int result;

	if (!stmt)
	{
		result = sqlite3_prepare_v2(db, "SELECT `number`, `text` FROM `traffregs` WHERE `rowid`=? LIMIT 1", -1, &stmt, NULL);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
	}

	result = sqlite3_reset(stmt);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_bind_int64(stmt, 1, id);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_step(stmt);
	if (result != SQLITE_ROW)
	{
		if (result != SQLITE_DONE)
		{
			g_error("traffreg: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
		return NULL;
	}

	gint32 number = sqlite3_column_int(stmt, 0);
	const gchar *text = (const gchar *)sqlite3_column_text(stmt, 1);

	return traffreg_new_with_id(id, number, text);
}

pdd_traffreg_t *traffreg_find_by_number(gint32 number)
{
	if (!use_cache)
	{
		gsize i;
		for (i = 0; i < get_traffregs()->len; i++)
		{
			pdd_traffreg_t *traffreg = g_ptr_array_index(get_traffregs(), i);
			if (traffreg->number == number)
			{
				return traffreg_copy(traffreg);
			}
		}
		return NULL;
	}

	static sqlite3_stmt *stmt = NULL;
	sqlite3 *db = database_get();
	int result;

	if (!stmt)
	{
		result = sqlite3_prepare_v2(db, "SELECT `rowid`, `text` FROM `traffregs` WHERE `number`=? LIMIT 1", -1, &stmt, NULL);
		if (result != SQLITE_OK)
		{
			g_error("traffreg: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
	}

	result = sqlite3_reset(stmt);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_bind_int(stmt, 1, number);
	if (result != SQLITE_OK)
	{
		g_error("traffreg: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
	}

	result = sqlite3_step(stmt);
	if (result != SQLITE_ROW)
	{
		if (result != SQLITE_DONE)
		{
			g_error("traffreg: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
		}
		return NULL;
	}

	gint64 id = sqlite3_column_int64(stmt, 0);
	const gchar *text = (const gchar *)sqlite3_column_text(stmt, 1);

	return traffreg_new_with_id(id, number, text);
}

pdd_traffregs_t *traffreg_copy_all(pdd_traffregs_t *traffregs)
{
	pdd_traffregs_t *traffregs_copy = g_ptr_array_new();
	gsize i;
	for (i = 0; i < traffregs->len; i++)
	{
		pdd_traffreg_t *traffreg = g_ptr_array_index(traffregs, i);
		g_ptr_array_add(traffregs_copy, traffreg_copy(traffreg));
	}
	return traffregs_copy;
}

void traffreg_free_all(pdd_traffregs_t *traffregs)
{
	g_ptr_array_foreach(traffregs, (GFunc)traffreg_free, NULL);
	g_ptr_array_free(traffregs, TRUE);
}
