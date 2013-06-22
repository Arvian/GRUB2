/* time.c - kernel time functions */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#include <grub/time.h>

typedef grub_uint64_t (*get_time_ms_func_t) (void);

/* Function pointer to the implementation in use.  */
static get_time_ms_func_t get_time_ms_func;

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
* @brief ʱ��ȡ�ú�����
*
* @note ע����ϸ����:
*
* ������ʵ��ʱ��ȡ�ú����Ĺ��ܡ���Ϊ��ͬ�ܹ���ʱ����㺯����һ���������ٵ���һ��ܹ��й�
* ��ʱ��ֵȡ�ú�����get_time_ms_func ��һ������ָ�룬ָ��ͬ�ܹ���ʱ��ֵȡ�ú�����
**/
grub_uint64_t
grub_get_time_ms (void)
{
  return get_time_ms_func ();
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
* @brief ʱ��ȡ�ú����İ�װ������
*
* @note ע����ϸ����:
*
* ������ʵ�ְ�װʱ��ȡ�ú���ָ��Ĺ��ܡ��趨ʱ��ֵȡ�ú���ָ�룬֮��ſ���͸������ָ��ִ
* ��ʱ��ֵȡ�ú�ʽ��ȡ��ϵͳʱ��ֵ�����磬grub-2.00/grub-core/kern/i386/tsc.c�о͵��ø�
* ����grub_install_get_time_ms (grub_tsc_get_time_ms)����װһ��ʹ��TSC��ʵ��ʱ��ȡ�ú�����
**/
void
grub_install_get_time_ms (get_time_ms_func_t func)
{
  get_time_ms_func = func;
}
