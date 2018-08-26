#pragma once
namespace HAL {
	class PN532 {
		public:
			PN532( void );
			PN532( uint8_t (*ptrToArrayOfUID)[10] );
			
		protected:
		private:
			uint8_t (*ptr_to_array_of_uid)[10];
	};
	
	class FRC522 {
		
	};
}
