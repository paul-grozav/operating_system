// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE_PCI_H
#define MODULE_PCI_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
//! All fields that describe a PCI device.
typedef struct
{
  /**
   * Identifies the manufacturer of the device. Where valid IDs are allocated by
   * PCI-SIG (the list is here: https://pcisig.com/membership/member-companies)
   * to ensure uniqueness and 0xFFFF is an invalid value that will be returned
   * on read accesses to Configuration Space registers of non-existent devices.
   */
  uint16_t vendor_id;

  /**
   * Identifies the particular device. Where valid IDs are allocated by the
   * vendor.
   */
  uint16_t device_id;

  /**
   * A value != 0 means true and a value of 0 means false.
   */
  uint8_t is_multifunction_device;

  /**
   * Only has a value > 0 for multifunction devices. Single function devices
   * will have a function value of 0.
   */
  uint8_t function;

  /**
   * A register used to record status information for PCI bus related events.
   */
  uint16_t status;

  /**
   * Provides control over a device's ability to generate and respond to PCI
   * cycles. Where the only functionality guaranteed to be supported by all
   * devices is, when a 0 is written to this register, the device is
   * disconnected from the PCI bus for all accesses except Configuration Space
   * access.
   */
  uint16_t command;

  /**
   * Specifies the type of function the device performs. See list here:
   * https://wiki.osdev.org/PCI#Class_Codes
   */
  uint8_t class_code;

  /**
   * Specifies the specific function the device performs. See list here:
   * https://wiki.osdev.org/PCI#Class_Codes
   */
  uint8_t subclass_code;

  /**
   * A read-only register that specifies a register-level programming interface
   * the device has, if it has any at all.
   */
  uint8_t prog_if;

  /**
   * Specifies a revision identifier for a particular device. Where valid IDs
   * are allocated by the vendor.
   */
  uint8_t revision_id;

  /**
   * Represents that status and allows control of a devices BIST (built-in self
   * test).
   */
  uint8_t bist;

  /**
   * Identifies the layout of the rest of the header beginning at byte 0x10 of
   * the header and also specifies whether or not the device has multiple
   * functions. Where a value of 0x00 specifies a general device, a value of
   * 0x01 specifies a PCI-to-PCI bridge, and a value of 0x02 specifies a CardBus
   * bridge. If bit 7 of this register is set, the device has multiple
   * functions; otherwise, it is a single function device.
   */
  uint8_t header_type;

  /**
   * Specifies the latency timer in units of PCI bus clocks.
   */
  uint8_t latency_timer;

  /**
   * Specifies the system cache line size in 32-bit units. A device can limit
   * the number of cacheline sizes it can support, if a unsupported value is
   * written to this field, the device will behave as if a value of 0 was
   * written.
   */
  uint8_t cache_line_size;
} module_pci_device_info;
// -------------------------------------------------------------------------- //
/**
 * Initialize with 0 or invalid values all the fields of the given structure.
 * @param[in] di - device info structure to be populated with default values.
 */
void module_pci_device_info_init(module_pci_device_info * const di);
// -------------------------------------------------------------------------- //
/**
 * Read 32 bits from the PCI Configuration Space. See
 * https://wiki.osdev.org/PCI#Header_Type_0x00 .
 * @param[in] bus - One of the 256 buses available.
 * @param[in] device - One of the 32 devices available per bus.
 * @param[in] function - Function of that device, if it is a multi function
 * device.
 * @param[in] offset - offset from where to read the bits/bytes.
 */
uint32_t pci_read(const uint8_t bus, const uint8_t device,
  const uint8_t function, const uint8_t offset);
// -------------------------------------------------------------------------- //
/**
 * Write 32 bits to the PCI Configuration Space. See
 * https://wiki.osdev.org/PCI#Header_Type_0x00 .
 * @param[in] bus - One of the 256 buses available.
 * @param[in] device - One of the 32 devices available per bus.
 * @param[in] function - Function of that device, if it is a multi function
 * device.
 * @param[in] offset - offset where to write the bits/bytes.
 * @param[in] data - 32 bits to be written.
 */
void pci_write(const uint8_t bus, const uint8_t device,
  const uint8_t function, const uint8_t offset, const uint32_t data);
// -------------------------------------------------------------------------- //
void module_pci_test();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

