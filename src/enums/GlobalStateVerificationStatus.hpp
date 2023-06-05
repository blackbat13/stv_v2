#ifndef GLOBAL_STATE_VERIFICATION_STATUS
#define GLOBAL_STATE_VERIFICATION_STATUS

enum GlobalStateVerificationStatus {
    UNVERIFIED, ///< State is not verified.
    PENDING, ///< Entered the state but it is not verified as correct or incorrect yet.
    VERIFIED_OK, ///< The state has been verified and is correct.
    VERIFIED_ERR, /// < The state has been verified and is incorrect.
};

#endif