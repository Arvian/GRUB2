/* main.c - the kernel main routine */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2005,2006,2008,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/kernel.h>
#include <grub/misc.h>
#include <grub/symbol.h>
#include <grub/dl.h>
#include <grub/term.h>
#include <grub/file.h>
#include <grub/device.h>
#include <grub/env.h>
#include <grub/mm.h>
#include <grub/command.h>
#include <grub/reader.h>
#include <grub/parser.h>

/* This is actualy platform-independant but used only on loongson and sparc.  */
#if defined (GRUB_MACHINE_MIPS_LOONGSON) || defined (GRUB_MACHINE_MIPS_QEMU_MIPS) || defined (GRUB_MACHINE_SPARC64)
/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief ���ģ������Ľ�����ַ��
*
* @note ע����ϸ����: 
*
* ������ʵ�ֻ�ȡģ������Ľ�����ַ�Ĺ��ܡ����grub_modbase��δ����ʼ������
* ֱ�ӷ���grub_modbase����(��ʾû��ģ���������У�������ַ�����ַ��ͬ)����
* ��grub_modbaseǿ��ת��Ϊgrub_module_info�ṹmodinfo������grub_modbase + 
* modinfo->size (˵����grub_modbase����ŵ���grub_module_info�ṹ��ÿ�����
* ģ��ʱ�����modinfo->size����Ӧʵ�ʵ�ģ������Ĵ�С)��
**/
grub_addr_t
grub_modules_get_end (void)
{
  struct grub_module_info *modinfo;

  modinfo = (struct grub_module_info *) grub_modbase;

  /* Check if there are any modules.  */
  if ((modinfo == 0) || modinfo->magic != GRUB_MODULE_MAGIC)
    return grub_modbase;

  return grub_modbase + modinfo->size;
}
#endif

/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief ����GRUB2���ĵ�����ELFģ�顣
*
* @note ע����ϸ����: 
*
* ������ʵ�ּ���GRUB2���ĵ�����ELFģ��Ĺ��ܡ�����FOR_MODULES����ÿ��ģ��,���
* ��ģ����OBJ_TYPE_ELF���͵ģ��͵���grub_dl_load_core()��ʵ�ʼ��أ��������ʹ�
* ӡ������Ϣ��
**/

/* Load all modules in core.  */
static void
grub_load_modules (void)
{
  struct grub_module_header *header;
  FOR_MODULES (header)
  {
    /* Not an ELF module, skip.  */
    if (header->type != OBJ_TYPE_ELF)
      continue;

    if (! grub_dl_load_core ((char *) header + sizeof (struct grub_module_header),
			     (header->size - sizeof (struct grub_module_header))))
      grub_fatal ("%s", grub_errmsg);

    if (grub_errno)
      grub_print_error ();
  }
}

/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief ���ز�ִ��GRUB2���ĵ�����ģ�顣
*
* @note ע����ϸ����: 
*
* ������ʵ�ּ���GRUB2���ĵ���������ģ��Ĺ��ܡ�����FOR_MODULES���ҵ���һ������
* ��OBJ_TYPE_CONFIG���͵�ģ�飬������grub_parser_execute()��ʵ�ʽ������ò�ִ��
* ��������Ȼ���˳���
**/
static void
grub_load_config (void)
{
  struct grub_module_header *header;
  FOR_MODULES (header)
  {
    /* Not an embedded config, skip.  */
    if (header->type != OBJ_TYPE_CONFIG)
      continue;
    
    grub_parser_execute ((char *) header +
			 sizeof (struct grub_module_header));
    break;
  }
}

/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief ɾ�����Ʋ���������(������ڵĻ�)��
*
* @note ע����ϸ����: 
*
* ������ʵ��ɾ�����Ʋ���val���ߵ����ŵĹ��ܡ��ú�����Ϊ������������д����
* (Write hook)��
**/

/* Write hook for the environment variables of root. Remove surrounding
   parentheses, if any.  */
static char *
grub_env_write_root (struct grub_env_var *var __attribute__ ((unused)),
		     const char *val)
{
  /* XXX Is it better to check the existence of the device?  */
  grub_size_t len = grub_strlen (val);

  if (val[0] == '(' && val[len - 1] == ')')
    return grub_strndup (val + 1, len - 2);

  return grub_strdup (val);
}

/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief ����GRUB2��prefix��root����������
*
* @note ע����ϸ����: 
*
* ������ʵ������GRUB2��prefix��root���������Ĺ��ܡ�
**/

static void
grub_set_prefix_and_root (void)
{
  char *device = NULL;
  char *path = NULL;
  char *fwdevice = NULL;
  char *fwpath = NULL;
  char *prefix = NULL;
  struct grub_module_header *header;

  FOR_MODULES (header)
    if (header->type == OBJ_TYPE_PREFIX)
      prefix = (char *) header + sizeof (struct grub_module_header);

  grub_register_variable_hook ("root", 0, grub_env_write_root);

  if (prefix)
    {
      char *pptr = NULL;
      if (prefix[0] == '(')
	{
	  pptr = grub_strrchr (prefix, ')');
	  if (pptr)
	    {
	      device = grub_strndup (prefix + 1, pptr - prefix - 1);
	      pptr++;
	    }
	}
      if (!pptr)
	pptr = prefix;
      if (pptr[0])
	path = grub_strdup (pptr);
    }
  if ((!device || device[0] == ',' || !device[0]) || !path)
    grub_machine_get_bootlocation (&fwdevice, &fwpath);

  if (!device && fwdevice)
    device = fwdevice;
  else if (fwdevice && (device[0] == ',' || !device[0]))
    {
      /* We have a partition, but still need to fill in the drive.  */
      char *comma, *new_device;

      for (comma = fwdevice; *comma; )
	{
	  if (comma[0] == '\\' && comma[1] == ',')
	    {
	      comma += 2;
	      continue;
	    }
	  if (*comma == ',')
	    break;
	  comma++;
	}
      if (*comma)
	{
	  char *drive = grub_strndup (fwdevice, comma - fwdevice);
	  new_device = grub_xasprintf ("%s%s", drive, device);
	  grub_free (drive);
	}
      else
	new_device = grub_xasprintf ("%s%s", fwdevice, device);

      grub_free (fwdevice);
      grub_free (device);
      device = new_device;
    }
  else
    grub_free (fwdevice);
  if (fwpath && !path)
    path = fwpath;
  else
    grub_free (fwpath);
  if (device)
    {
      char *prefix_set;
    
      prefix_set = grub_xasprintf ("(%s)%s", device, path ? : "");
      if (prefix_set)
	{
	  grub_env_set ("prefix", prefix_set);
	  grub_free (prefix_set);
	}
      grub_env_set ("root", device);
    }

  grub_free (device);
  grub_free (path);
  grub_print_error ();
}

/* Load the normal mode module and execute the normal mode if possible.  */
static void
grub_load_normal_mode (void)
{
  /* Load the module.  */
  grub_dl_load ("normal");

  /* Print errors if any.  */
  grub_print_error ();
  grub_errno = 0;

  grub_command_execute ("normal", 0, 0);
}

/**
* @attention ��ע�͵õ���"�˸߻�"�Ƽ��ش�ר��2012����⡰��Դ����ϵͳ�ں˷����Ͱ�ȫ������
*�������ţ�2012ZX01039-004������������
*
* @copyright ע����ӵ�λ���廪��ѧ����03����Linux�ں����ͨ�û���������������е���λ
*
* @author ע�������Ա��л��ѧ
*
* @date ע��������ڣ�2013��6��8��
*
* @brief GRUB2����������ڡ�
*
* @note ע����ϸ����:
*
* grub_main()��GRUB2�ĺ��Ĺ���ѭ���������Ըú���ѭ���ķ��������Եó�GRUB�ĺ���
* ����ܹ���
* 
* 1) ע�ᵼ������
*    
*    grub_register_exported_symbols() ����ͨ��gensymlist.sh�ű���Ԥ����׶�ɨ��
*    ����ͷ�ļ����ҳ�����ΪEXPORT_FUNC����EXPORT_VAR�ĺ������߱���������һ����
*    ���ķ��ű����顣Ȼ��Ը������ÿ�����grub_dl_register_symbol()����ע�ᡣ
*    grub_dl_register_symbol()ͨ���Է������ֽ���hash�������÷���ӳ�䵽
*    grub_symtab[]����Ķ�Ӧ���С�
*
* 2) ���غ���ģ��
* 
*    ���ڳ�ʼ�����̶Ե����׶ν�ѹ�������У�ʵ�����Ѿ���GRUB kernel�͸�ģ�鶼��
*    ѹ����GRUB_MEMORY_MACHINE_DECOMPRESSION_ADDR ��ʼ����Ҳ����0x100000������
*    kernel�����Ѿ������ƻ������ӵ�ַ0x9000��������ģ�鲿�ֻ���ԭ������ѹ����
*    �ط���grub_load_modules() �������Ǵ�ģ�鲿�ּ��ظ�ģ�顣�ò�����Ҫ��ͨ��
*    FOR_MODULES��ɨ�����е�ģ��ͷ����ͨ��grub_dl_load_core()������ʵ��ģ���ʵ
*    �ʼ��ء�����֮������ģ��ĳ�ʼ������������ģ�����ӽ�������
*
* 3) ע���������
*
*    grub_register_core_commands()����grub_register_command()�ֱ�ע�����¼�����
*    ������: 
*
*    - ����	        ����	            ������
*
*    - set	        ���û�������ֵ	    grub_core_cmd_set()
*    - unset	    ɾ����������	    grub_core_cmd_unset()
*    - ls	        �г��豸�����ļ�	grub_core_cmd_ls()
*    - insmod	    ����ģ��	        grub_core_cmd_insmod()
*
*    grub_register_command()ʵ���ϵ��õ���grub_register_command_prio()��������
*    ���뵽grub_command_list�б��С�
*
* 4) ������Ƕ����
*
*    grub_load_config()������ģ������ÿ������OBJ_TYPE_CONFIG���͵�ģ����ú���
*    grub_parser_execute()��ɨ�������ģ���ڵ�ÿһ�У��������У��ҵ���Ӧ���
*    ��ִ����������Щ����ģ����Ƕ�뵽ģ�����ڵĶ�GRUB�ĳ�ʼ�����á�
*
* 5) ִ�г���ģʽ
*
*    grub_load_normal_mode()ͨ������grub_dl_load()������Ϊ"normal"��ģ�飬��ͨ
*    ��grub_command_execute()���и�ģ�顣
*
*    ���У�grub_dl_load()��������ͨ��grub_dl_get()�鿴�Ƿ��ж�Ӧ���ֵ�ģ���Ѿ�
*    �����ص�����grub_dl_headΪ�׵�ģ�������У�����еĻ�����ֱ�ӷ��ظ�ģ�顣
*    ����grub_dl_load()��������GRUB�İ�װĿ¼��libĿ¼(grub_dl_dir)�¶�Ӧ
*    CPUƽ̨��GRUB_TARGET_CPU�Լ�GRUB_PLATFORM��Ŀ¼�£�Ѱ�Ҷ�Ӧ���ֵ�*.modģ
*    �飬��ͨ��grub_dl_load_file()����������grub_dl_load_core()��ģ����ؽ���
*    grub_dl_headΪ�׵�ģ�������С�
*
*    ��grub_command_execute()����ͨ��grub_command_find()��grub_command_list��
*    ���ϲ��Ҷ�Ӧ������ҵ������ִ��֮��
*
*    ��ˣ������������Ϊ"normal"��ģ���ִ�У�����֮ǰ��û���������ģ�飬��
*    �˱ض����ȴ�CPUƽ̨������lib/i386-pc/������ȥѰ��normal.mod��Ȼ����ؽ���
*    grub_dl_head������ִ�и�ģ��ĳ�ʼ��������"normal"ģ����GRUB�ĵ�һ��ִ��
*    ��ģ�飬������ģ�鶼���������������±���һ������ִ�С�"normal"ģ��Դ����λ
*    ��grub-core/normalĿ¼�£��۲��Ŀ¼�µ��ļ������Է�����grub-core/normal/main.c��
*    ����"normal"ģ�飬���ʼ���������Ǹ��ļ��е�GRUB_MOD_INIT(normal)����Ӧ��
*    ������
*
*    GRUB_MOD_INIT(normal)����Ӧ�ĺ�����ִ��һЩ�ؼ��������������grub_dl_load()
*    ���ع��õ�"gzio"ģ�飬����һЩ�Ӻ���ע�����йؼ����
*
*    - ����	            ����	                        ������
*
*    - authenticate	    ����û��Ƿ���USERLIST֮��	    grub_cmd_authenticate ()
*    - export	        ��������	                    grub_cmd_export ()
*    - break	        ����ѭ��	                    grub_script_break ()
*    - continue	        ����ѭ��	                    grub_script_break ()
*    - shift	        �ƶ���λ������$0, $1, $2, ...��	grub_script_shift()
*    - setparams	    ���ö�λ����	                grub_script_setparams()
*    - return	        ��һ����������(��bash����һ��)  grub_script_return()
*    - menuentry	    ����һ���˵���	                grub_cmd_menuentry()
*    - submenu	        ����һ���Ӳ˵�	                grub_cmd_menuentry()
*    - clear	        ����Ļ	                        grub_mini_cmd_clear()
*    - normal	        ����normalģʽ	                grub_cmd_normal()
*    - normal_exit	    �˳�normalģʽ	                grub_cmd_normal_exit()
*
*    ������normalģ��ĳ�ʼ���������Ѿ�ע����normal��������grub_command_execute()
*    ʱ������grub_command_list�б��ϲ��ҵ�������Ӷ�ִ��grub_cmd_normal()������
*    ����������У������ҵ�һ�������ļ�������grub.cfg��֮����ú���
*    grub_enter_normal_mode()������GRUB�ĳ���ģʽ��
*
*    grub_cmd_normal()�����᳢�Զ��벢ִ�������ļ�(���簲װĿ¼�µ�grub.cfg)��
*    �������û���������ʱ���õ����÷�ʽ���������ָ��Ҫ�����Ĳ���ϵͳ�Ĳ˵�����
*    ��ѡ��������һ����
*
*    ���Ž���grub_cmdline_run()������һ��while (1)ѭ������ȡ�û����룬���Ͳ�ִ��
*   ��ͨ��grub_script_execute()������ѭ��ֱ���û�����normal_exit�����ͨ��
*    grub_cmd_normal_exit()������
*
* 6) �����Ԯģʽ
*
*    ���ǰһ����"normal"��ģ��ִ���˳�����GRUB�����Ԯģʽ(rescue mode)����ִ
*    �к���grub_rescue_run()���ú���ʽ��while(1)ѭ�����������϶�ȡ�û���������
*    �룬���������ִ��֮�����ж��û�����������������ͨ��grub_command_find()
*    ��ɵġ��ú�����Զ���᷵�أ����GRUBҲ�Ͳ��᷵�ء�
**/

/* The main routine.  */
void __attribute__ ((noreturn))
grub_main (void)
{
  /* First of all, initialize the machine.  */
  grub_machine_init ();

  /* Hello.  */
  grub_setcolorstate (GRUB_TERM_COLOR_HIGHLIGHT);
  grub_printf ("Welcome to GRUB!\n\n");
  grub_setcolorstate (GRUB_TERM_COLOR_STANDARD);

  /* Load pre-loaded modules and free the space.  */
  grub_register_exported_symbols ();
#ifdef GRUB_LINKER_HAVE_INIT
  grub_arch_dl_init_linker ();
#endif  
  grub_load_modules ();

  /* It is better to set the root device as soon as possible,
     for convenience.  */
  grub_set_prefix_and_root ();
  grub_env_export ("root");
  grub_env_export ("prefix");

  grub_register_core_commands ();

  grub_load_config ();
  grub_load_normal_mode ();
  grub_rescue_run ();
}
