#!/usr/bin/perl

use strict;

# Read input and generate pin definitions
open( INPUT , "pins.csv" ) or die;
my @pinDefs = ( );
my %byPort = ( );
my %fullMap = ( );
while ( <INPUT> ) {
	chop;
	my ($chip,$gpio) = split /\s+/;
	my ($port,$pad);
	$port = $pad = $gpio;

	$port =~ s/^P(.).+/GPIO$1/;
	$pad =~ s/^[^\d]+//;

	$fullMap{$chip} = [ $port , $pad ];
	$byPort{$port} = { } unless exists $byPort{$port};
	$byPort{$port}->{$chip} = $pad;

	push @pinDefs, "#define SSD1963_$chip\_PORT\t$port";
	push @pinDefs, "#define SSD1963_$chip\_PAD\t$pad";
}
close INPUT;

open( OUTPUT , '>ssd1963_pins.h' ) or die;
print OUTPUT <<'EOF';
#ifndef _H_PINS_SSD1963_STM32F4
#define _H_PINS_SSD1963_STM32F4


// Pin definitions
EOF

print OUTPUT join("\n" , @pinDefs) . "\n";

# Generate initialisation code
print OUTPUT <<'EOF';

// Initialise all GPIO ports used by the SSD1963
#define _ssd1963_init_gpio(spd) \
	do { \
EOF
foreach my $port ( keys %byPort ) {
	my $mask = 0;
	foreach my $chip ( keys %{ $byPort{$port} } ) {
		$mask |= ( 1 << $fullMap{$chip}->[1] );
	}
	$mask = sprintf "0x%0.4x" , $mask;
	print OUTPUT "\t\tpalSetGroupMode( $port , $mask , 0 , \\\n"
	. "\t\t\tPAL_MODE_OUTPUT_PUSHPULL | spd ); \\\n";
}
print OUTPUT "\t} while ( 0 )\n";

# Generate macros for the various state pins
my @states = ( 'RESET' , 'CS' , 'RD' , 'WR' , 'RS' );
foreach my $state ( @states ) {
	print OUTPUT "\n";
	print OUTPUT "#define _ssd1963_set_" . lc( $state )
		. " \\\n\tpalSetPad( " . $fullMap{$state}->[0] . " , "
		. $fullMap{$state}->[1] . " )\n";
	print OUTPUT "#define _ssd1963_clear_" . lc( $state )
		. " \\\n\tpalClearPad( " . $fullMap{$state}->[0] . " , "
		. $fullMap{$state}->[1] . " )\n";
		
}

# Generate the macro that sets all data pins
#	For each GPIO port
#		Find all Dx pins on that port
print OUTPUT <<'EOF';

// Write to the SSD1963's data ports
#define _ssd1963_write(in) \
	do { \
EOF
foreach my $port ( keys %byPort ) {
	my %dataPads = ( );
	for ( my $d = 0 ; $d < 16 ; $d ++ ) {
		$dataPads{$d} = $byPort{$port}->{"D$d"}
			if exists $byPort{$port}->{"D$d"};
	}
	next unless %dataPads;

	my $pStart = -1;
	my ( $bEnd , $bStart ,$bit );
	my @sequences = ( );
	for ( $bit = 0 ; $bit < 16 ; $bit ++ ) {
		if ( exists $dataPads{$bit} ) {
			if ( $pStart == -1 ) {
				$pStart = $dataPads{$bit};
				$bEnd = $bit;
				$bStart = $bit;
				next;
			} elsif ( $dataPads{$bit} ==  $dataPads{$bEnd} + 1 ) {
				$bEnd = $bit;
				next;
			}
		}
		if ( $pStart != -1 ) {
			push @sequences , {
				startPad => $pStart ,
				startBit => $bStart ,
				nBits    => $bEnd - $bStart + 1
			};
			$pStart = -1;
			$bit --;
		}
	}
	if ( $pStart != -1 ) {
		push @sequences , {
			startPad => $pStart ,
			startBit => $bStart ,
			nBits    => $bEnd - $bStart + 1
		};
	}

	my $outputMask = 0;
	my @parts = ( );
	foreach my $seq ( @sequences ) {
		my $mask = 0;
		for ( my $i = 0 ; $i < $seq->{nBits} ; $i ++ ) {
			$mask <<= 1;
			$mask |= 1;
		}

		my $padMask = $mask << $seq->{startPad};
		$outputMask |= $mask << $seq->{startPad};

		my $inputGet = "(in)";
		if ( $seq->{startBit} > 0 ) {
			$inputGet = "( $inputGet >> " . $seq->{startBit} . " )";
		}
		$inputGet = "( $inputGet & " . sprintf( '0x%0.4x' , $mask )
			. " )";
		if ( $seq->{startPad} > 0 ) {
			$inputGet = "( $inputGet << " . $seq->{startPad} . " )";
		}
		push @parts , $inputGet;
	}
	print OUTPUT "\t\tpalWritePort( $port , ( palReadLatch( $port ) & ~( "
		. sprintf( '0x%0.4x' , $outputMask ) . " ) ) \\\n\t\t\t| "
		. join(" \\\n\t\t\t| " , @parts ) . " ); \\\n";
}

print OUTPUT <<'EOF';
	} while ( 0 )

#endif //_H_PINS_SSD1963_STM32F4
EOF
