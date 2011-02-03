#include "question.h"
#include "config.h"
#include "database.h"
#include "settings.h"
#include "topic.h"

#include <stdlib.h>

gint *ticket_topics_distribution = NULL;

static pdd_question_t *question_new_with_id(gint64 id, gint64 topic_id, const gchar *text, gint64 image_id, const gchar *advice, gint64 comment_id)
{
    pdd_question_t *question = g_new(pdd_question_t, 1);
    question->id = id;
    question->topic_id = topic_id;
    question->text = g_strdup(text);
    question->image_id = image_id;
    question->advice = g_strdup(advice);
    question->comment_id = comment_id;
    return question;
}

pdd_question_t *question_new(gint64 topic_id, const gchar *text, gint64 image_id, const gchar *advice, gint64 comment_id)
{
    return question_new_with_id(0, topic_id, text, image_id, advice, comment_id);
}

void question_free(pdd_question_t *question)
{
    g_free(question->text);
    g_free(question->advice);
    g_free(question);
}

gboolean question_save(pdd_question_t *question)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "INSERT INTO `questions` (`topic_id`, `text`, `image_id`, `advice`, `comment_id`) VALUES (?, ?, ?, ?, ?)", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    result = sqlite3_reset(stmt);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = question->topic_id ? sqlite3_bind_int64(stmt, 1, question->topic_id) : sqlite3_bind_null(stmt, 1);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_text(stmt, 2, question->text, -1, NULL);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = question->image_id ? sqlite3_bind_int64(stmt, 3, question->image_id) : sqlite3_bind_null(stmt, 3);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_text(stmt, 4, question->advice, -1, NULL);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = question->comment_id ? sqlite3_bind_int64(stmt, 5, question->comment_id) : sqlite3_bind_null(stmt, 5);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE)
    {
        g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    question->id = sqlite3_last_insert_rowid(db);

    return TRUE;
}

gboolean question_set_sections(pdd_question_t *question, pdd_sections_t *sections)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "INSERT INTO `questions_sections` (`question_id`, `section_id`) VALUES (?, ?)", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    gsize i;
    for (i = 0; i < sections->len; i++)
    {
        pdd_section_t *section = ((pdd_section_t **)sections->pdata)[i];

        result = sqlite3_reset(stmt);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_bind_int64(stmt, 1, question->id);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_bind_int64(stmt, 2, section->id);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_step(stmt);
        if (result != SQLITE_DONE)
        {
            g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    return TRUE;
}

gboolean question_set_traffregs(pdd_question_t *question, pdd_traffregs_t *traffregs)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "INSERT INTO `questions_traffregs` (`question_id`, `traffreg_id`) VALUES (?, ?)", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    gsize i;
    for (i = 0; i < traffregs->len; i++)
    {
        pdd_traffreg_t *traffreg = ((pdd_traffreg_t **)traffregs->pdata)[i];

        result = sqlite3_reset(stmt);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_bind_int64(stmt, 1, question->id);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_bind_int64(stmt, 2, traffreg->id);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        result = sqlite3_step(stmt);
        if (result != SQLITE_DONE)
        {
            g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    return TRUE;
}

pdd_question_t *question_find_by_id(gint64 id)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "SELECT `topic_id`, `text`, `image_id`, `advice`, `comment_id` FROM `questions` WHERE `rowid`=? LIMIT 1", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    result = sqlite3_reset(stmt);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_int64(stmt, 1, id);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW)
    {
        if (result != SQLITE_DONE)
        {
            g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
        return NULL;
    }

    gint64 topic_id = sqlite3_column_int64(stmt, 0);
    const gchar *text = (const gchar *)sqlite3_column_text(stmt, 1);
    gint64 image_id = sqlite3_column_int64(stmt, 2);
    const gchar *advice = (const gchar *)sqlite3_column_text(stmt, 3);
    gint64 comment_id = sqlite3_column_int64(stmt, 4);

    return question_new_with_id(id, topic_id, text, image_id, advice, comment_id);
}

pdd_questions_t *question_find_by_section(gint64 section_id)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "SELECT q.`rowid`, q.`topic_id`, q.`text`, q.`image_id`, q.`advice`, q.`comment_id` FROM `questions` q INNER JOIN `questions_sections` qs ON q.`rowid`=qs.`question_id` WHERE qs.`section_id`=?", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    result = sqlite3_reset(stmt);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_int64(stmt, 1, section_id);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    pdd_questions_t *questions = g_ptr_array_new();

    while (TRUE)
    {
        result = sqlite3_step(stmt);
        if (result == SQLITE_DONE)
        {
            break;
        }
        if (result != SQLITE_ROW)
        {
            g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        gint64 id = sqlite3_column_int64(stmt, 0);
        gint64 topic_id = sqlite3_column_int64(stmt, 1);
        const gchar *text = (const gchar *)sqlite3_column_text(stmt, 2);
        gint64 image_id = sqlite3_column_int64(stmt, 3);
        const gchar *advice = (const gchar *)sqlite3_column_text(stmt, 4);
        gint64 comment_id = sqlite3_column_int64(stmt, 5);

        g_ptr_array_add(questions, question_new_with_id(id, topic_id, text, image_id, advice, comment_id));
    }

    return questions;
}

pdd_questions_t *question_find_with_offset(gint64 topic_id, gint offset, gint count)
{
    static sqlite3_stmt *stmt = NULL;
    sqlite3 *db = database_get();
    int result;

    if (!stmt)
    {
        result = sqlite3_prepare_v2(db, "SELECT `rowid`, `text`, `image_id`, `advice`, `comment_id` FROM `questions` WHERE `topic_id`=? LIMIT ?,?", -1, &stmt, NULL);
        if (result != SQLITE_OK)
        {
            g_error("question: unable to prepare statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }
    }

    result = sqlite3_reset(stmt);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to reset prepared statement (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_int64(stmt, 1, topic_id);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_int(stmt, 2, offset);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    result = sqlite3_bind_int(stmt, 3, count);
    if (result != SQLITE_OK)
    {
        g_error("question: unable to bind param (%d: %s)\n", result, sqlite3_errmsg(db));
    }

    pdd_questions_t *questions = g_ptr_array_new();

    while (TRUE)
    {
        result = sqlite3_step(stmt);
        if (result == SQLITE_DONE)
        {
            break;
        }
        if (result != SQLITE_ROW)
        {
            g_error("question: unable to perform statement (%d: %s)\n", result, sqlite3_errmsg(db));
        }

        gint64 id = sqlite3_column_int64(stmt, 0);
        const gchar *text = (const gchar *)sqlite3_column_text(stmt, 1);
        gint64 image_id = sqlite3_column_int64(stmt, 2);
        const gchar *advice = (const gchar *)sqlite3_column_text(stmt, 3);
        gint64 comment_id = sqlite3_column_int64(stmt, 4);

        g_ptr_array_add(questions, question_new_with_id(id, topic_id, text, image_id, advice, comment_id));
    }

    return questions;
}

pdd_questions_t *question_find_by_topic(gint64 topic_id, gint ticket_number)
{
    if (ticket_number > 0)
    {
        return question_find_with_offset(topic_id, (ticket_number - 1) * 10, 10);
    }
    return question_find_with_offset(topic_id, 0, -1);
}

static void load_ticket_topics_distribution()
{
    if (ticket_topics_distribution)
    {
        return;
    }

    gchar *raw_ttd = get_settings("ticket_topics_distribution");
    gchar **ttd = g_strsplit(raw_ttd, ":", 0);
    g_free(raw_ttd);

    ticket_topics_distribution = g_new(gint, g_strv_length(ttd));

    gchar **it = ttd;
    gsize i = 0;
    while (*it)
    {
        ticket_topics_distribution[i] = atoi(*it);
        it++;
        i++;
    }

    g_strfreev(ttd);
}

pdd_questions_t *question_find_by_ticket(gint ticket_number)
{
    load_ticket_topics_distribution();

    const pdd_topics_t *topics = topic_find_all();
    gsize i = 0;
    gint j;
    pdd_questions_t *questions = g_ptr_array_new();
    for (i = 0; i < topics->len; i++)
    {
        const pdd_topic_t *topic = g_ptr_array_index(topics, i);
        gint32 count = topic_get_question_count(topic);
        for (j = 0; j < ticket_topics_distribution[i]; j++)
        {
            pdd_questions_t *topic_questions = question_find_with_offset(topic->id, ((ticket_number - 1) * 10 + j) % count, 1);
            g_ptr_array_add(questions, g_ptr_array_index(topic_questions, 0));
            g_ptr_array_free(topic_questions, TRUE);
        }
    }
    return questions;
}

pdd_questions_t *question_find_random()
{
    load_ticket_topics_distribution();

    const pdd_topics_t *topics = topic_find_all();
    gsize i;
    gint j;
    pdd_questions_t *questions = g_ptr_array_new();
    for (i = 0; i < topics->len; i++)
    {
        const pdd_topic_t *topic = g_ptr_array_index(topics, i);
        gint32 count = topic_get_question_count(topic);
        for (j = 0; j < ticket_topics_distribution[i]; j++)
        {
            pdd_questions_t *topic_questions = question_find_with_offset(topic->id, g_random_int_range(0, count - 1), 1);
            g_ptr_array_add(questions, g_ptr_array_index(topic_questions, 0));
            g_ptr_array_free(topic_questions, TRUE);
        }
    }
    return questions;
}

void question_free_all(pdd_questions_t *questions)
{
    g_ptr_array_foreach(questions, (GFunc)question_free, NULL);
    g_ptr_array_free(questions, TRUE);
}