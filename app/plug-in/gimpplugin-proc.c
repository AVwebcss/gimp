/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpplugin-proc.c
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gegl.h>

#include "libgimpbase/gimpbase.h"

#include "plug-in-types.h"

#include "core/gimp.h"

#include "gimpplugin.h"
#include "gimpplugin-proc.h"
#include "gimpplugindef.h"
#include "gimppluginmanager.h"
#include "gimptemporaryprocedure.h"

#include "gimp-intl.h"


/*  local function prototypes  */

static GimpPlugInProcedure * gimp_plug_in_proc_find (GimpPlugIn  *plug_in,
                                                     const gchar *proc_name);


/*  public functions  */

gboolean
gimp_plug_in_set_proc_image_types (GimpPlugIn  *plug_in,
                                   const gchar *proc_name,
                                   const gchar *image_types)
{
  GimpPlugInProcedure *proc;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register images types "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    proc_name);

      return FALSE;
    }

  gimp_plug_in_procedure_set_image_types (proc, image_types);

  return TRUE;
}

gboolean
gimp_plug_in_set_proc_menu_label (GimpPlugIn  *plug_in,
                                  const gchar *proc_name,
                                  const gchar *menu_label)
{
  GimpPlugInProcedure *proc;
  GError              *error = NULL;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);
  g_return_val_if_fail (menu_label != NULL && strlen (menu_label), FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the menu label \"%s\" "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    menu_label, proc_name);

      return FALSE;
    }

  if (! gimp_plug_in_procedure_set_menu_label (proc, menu_label, &error))
    {
      gimp_message_literal (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                            error->message);
      g_clear_error (&error);

      return FALSE;
    }

  return TRUE;
}

gboolean
gimp_plug_in_add_proc_menu_path (GimpPlugIn  *plug_in,
                                 const gchar *proc_name,
                                 const gchar *menu_path)
{
  GimpPlugInProcedure *proc;
  GError              *error = NULL;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);
  g_return_val_if_fail (menu_path != NULL, FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the menu item \"%s\" "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    menu_path, proc_name);

      return FALSE;
    }

  if (! gimp_plug_in_procedure_add_menu_path (proc, menu_path, &error))
    {
      gimp_message_literal (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                            error->message);
      g_clear_error (&error);

      return FALSE;
    }

  return TRUE;
}

gboolean
gimp_plug_in_set_proc_icon (GimpPlugIn   *plug_in,
                            const gchar  *proc_name,
                            GimpIconType  type,
                            const guint8 *data,
                            gint          data_length)
{
  GimpPlugInProcedure *proc;
  GError              *error = NULL;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to set the icon "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    proc_name);

      return FALSE;
    }

  if (! gimp_plug_in_procedure_set_icon (proc, type, data, data_length,
                                         &error))
    {
      gimp_message_literal (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                            error->message);
      g_clear_error (&error);

      return FALSE;
    }

  return TRUE;
}

gboolean
gimp_plug_in_set_proc_help (GimpPlugIn  *plug_in,
                            const gchar *proc_name,
                            const gchar *blurb,
                            const gchar *help,
                            const gchar *help_id)
{
  GimpPlugInProcedure *proc;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register help "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    proc_name);

      return FALSE;
    }

  gimp_procedure_set_help (GIMP_PROCEDURE (proc),
                           blurb, help, help_id);

  return TRUE;
}

gboolean
gimp_plug_in_set_proc_attribution (GimpPlugIn  *plug_in,
                                   const gchar *proc_name,
                                   const gchar *authors,
                                   const gchar *copyright,
                                   const gchar *date)
{
  GimpPlugInProcedure *proc;

  g_return_val_if_fail (GIMP_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (proc_name != NULL, FALSE);

  proc = gimp_plug_in_proc_find (plug_in, proc_name);

  if (! proc)
    {
      gimp_message (plug_in->manager->gimp, NULL, GIMP_MESSAGE_ERROR,
                    "Plug-in \"%s\"\n(%s)\n"
                    "attempted to register the attribution "
                    "for procedure \"%s\".\n"
                    "It has however not installed that procedure. "
                    "This is not allowed.",
                    gimp_object_get_name (plug_in),
                    gimp_file_get_utf8_name (plug_in->file),
                    proc_name);

      return FALSE;
    }

  gimp_procedure_set_attribution (GIMP_PROCEDURE (proc),
                                  authors, copyright, date);

  return TRUE;
}

/*  private functions  */

static GimpPlugInProcedure *
gimp_plug_in_proc_find (GimpPlugIn  *plug_in,
                        const gchar *proc_name)
{
  GimpPlugInProcedure *proc = NULL;

  if (plug_in->plug_in_def)
    proc = gimp_plug_in_procedure_find (plug_in->plug_in_def->procedures,
                                        proc_name);

  if (! proc)
    proc = gimp_plug_in_procedure_find (plug_in->temp_procedures, proc_name);

  return proc;
}