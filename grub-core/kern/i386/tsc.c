/* kern/i386/tsc.c - x86 TSC time source implementation
 * Requires Pentium or better x86 CPU that supports the RDTSC instruction.
 * This module uses the RTC (via grub_get_rtc()) to calibrate the TSC to
 * real time.
 *
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

#include <grub/types.h>
#include <grub/time.h>
#include <grub/misc.h>
#include <grub/i386/tsc.h>
#include <grub/i386/pit.h>

/* This defines the value TSC had at the epoch (that is, when we calibrated it). */
static grub_uint64_t tsc_boot_time;

/* Calibrated TSC rate.  (In TSC ticks per millisecond.) */
static grub_uint64_t tsc_ticks_per_ms;

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
* @brief �ض���i386-pc�ܹ���TSCʱ��ȡ�ú�����
*
* @note ע����ϸ����:
*
* ������ʵ���ض���i386-pc�ܹ���TSCʱ��ȡ�ú����Ĺ��ܡ�����I386-PC �ܹ����е�ʱ��ȡ�ú�����
* ȡ��CPU ʱ��ֵ����ת���ɺ���Ϊ��λ��ʱ��ֵ�����к���grub_get_tsc() ȡ��CPUʱ�����ֵ��
* tsc_ticks_per_ms ��ʱ�����ֵת���ɺ�ʱ��ֵ�ĳ���ֵ��
**/
grub_uint64_t
grub_tsc_get_time_ms (void)
{
  return tsc_boot_time + grub_divmod64 (grub_get_tsc (), tsc_ticks_per_ms, 0);
}


/* How many RTC ticks to use for calibration loop. (>= 1) */
#define CALIBRATION_TICKS 2
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
* @brief ����RTC��ʱ���У׼������
*
* @note ע����ϸ����:
*
* ������ʵ�ֻ���RTC��ʱ���У׼�����Ĺ��ܡ���������ȡ��CPUʱ���ֵ������ʱ����ʼֵ��Ȼ��
* ʹ��PIT�ȴ�����0xffff��ʱ��ֵ;�����ٴ�ȡ��CPUʱ���ֵ������ʱ������ֵ�����ʱ����Ĳ���
* ����ʱ��ֵ55���룬���ÿ�����ʱ�����ʱ����������tsc_ticks_per_ms�С�
**/
/* Calibrate the TSC based on the RTC.  */
static void
calibrate_tsc (void)
{
  /* First calibrate the TSC rate (relative, not absolute time). */
  grub_uint64_t start_tsc;
  grub_uint64_t end_tsc;

  start_tsc = grub_get_tsc ();
  grub_pit_wait (0xffff);
  end_tsc = grub_get_tsc ();

  tsc_ticks_per_ms = grub_divmod64 (end_tsc - start_tsc, 55, 0);
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
* @brief ʱ�����ʼ��������
*
* @note ע����ϸ����:
*
* ������ʵ��ʱ�����ʼ�������Ĺ��ܡ�������CPU֧��ʱ���ʱ��ȡ��Ŀǰʱ�����ֵ������ʱ��
* ��ʼֵ��Ȼ�����ʱ��У׼����calibrate_tsc()��ȡ�ú���ʱ�����ʱ����ֵ��Ȼ��װʱ���
* ��ʱ��ȡ�ú�ʽ��Ϊʱ���������ʱ��ȡ�ú��������CPUû��֧��ʱ�����ʹ��RTC��ʱ��ȡ��
* ����grub_rtc_get_time_ms��Ϊʱ���������ʱ��ȡ�ú������ú��������жϻ����Ƿ�֧��TSC��
* ʱ�����������TSC����һ��64λ�Ĵ��������������б���������x86�����������Ӹ�λ��ʼ�Ͷ�
* ���������������м�����ָ��RDTSC����TSC��ֵ�� EDX:EAX�С���x86-64ģʽʱ��RDTSCҲ���
* RAX�ĸ�32λ��ʹ��ʱ�����������TSC���������Գ�ɫ�ĸ߷ֱ��ʣ��Ϳ����ķ�ʽ���CPUʱ��
* ��Ϣ����ˣ��������֧��TSC���ͻᾡ��ʹ��TSC��Ϊ��ʱ��ʽ����ʱ����һ���¾��ǵ���
* grub_get_tsc ()�����GRUB2������ʱ��tsc_boot_time�����ţ�����calibrate_tsc ()��У��
* TSC�����tsc_ticks_per_ms����ʾû����TSC��������ٴΣ����У����ͨ��PIT�����еģ���
**/
void
grub_tsc_init (void)
{
  if (grub_cpu_is_tsc_supported ())
    {
      tsc_boot_time = grub_get_tsc ();
      calibrate_tsc ();
      grub_install_get_time_ms (grub_tsc_get_time_ms);
    }
  else
    {
#if defined (GRUB_MACHINE_PCBIOS) || defined (GRUB_MACHINE_IEEE1275)
      grub_install_get_time_ms (grub_rtc_get_time_ms);
#else
      grub_fatal ("no TSC found");
#endif
    }
}
